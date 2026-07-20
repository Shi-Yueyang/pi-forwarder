#include "forwarder.hpp"
#include "ini_generator.hpp"
#include "log.hpp"

#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace forwarder {

// ============================================================================
// Construction / Destruction
// ============================================================================

Forwarder::Forwarder(const Config& config)
    : config_(config)
{
    LOG_INFO("pi_forwarder initializing (Phase 5)");

    create_sockets();
    start_receive_loops();

    // Generate the RSSP1 INI and initialize the stack
    std::string ini = generate_ini_string(config_);
    LOG_DEBUG("Generated RSSP1 INI:\n" + ini);

    if (!rssp1_.init(ini)) {
        throw std::runtime_error("RSSP1 stack initialization failed");
    }

    // Sync RSSP1 log level with our own
    // RSSP1 levels: 0=off, 1=error, 2=warning, 3=info, 4=debug
    {
        std::uint16_t rssp1_level = 3; // default: info
        switch (config_.log_level) {
            case LogLevel::Trace: rssp1_level = 4; break;
            case LogLevel::Debug: rssp1_level = 4; break;
            case LogLevel::Info:  rssp1_level = 3; break;
            case LogLevel::Warn:  rssp1_level = 2; break;
            case LogLevel::Error: rssp1_level = 1; break;
        }
        rssp1_.set_log_level(rssp1_level);
    }

    // ---- Construct data-path processors ----
    rssp1_to_udp_ = std::make_unique<Rssp1ToUdp>(
        rx_frame_queue_, rssp1_, config_,
        [this](int conn_idx, std::vector<std::uint8_t> data) {
            send_to_local_peer(conn_idx, data);
        });

    udp_to_rssp1_ = std::make_unique<UdpToRsp1>(
        tx_payload_queue_, rssp1_, config_,
        [this](int conn_idx, int chn_idx,
               std::vector<std::uint8_t> data,
               std::uint32_t ip, std::uint16_t port) {
            // Skip frames with no valid destination (e.g. during RSSP1 startup
            // before the connection is fully established)
            if (ip == 0 || port == 0) {
                LOG_DEBUG("Send frame: skipping (ip=0x" +
                          std::to_string(ip) + ", port=" + std::to_string(port) +
                          ") — connection not yet established");
                return;
            }

            // Resolve (conn_idx, chn_idx) to flat peer socket index
            if (conn_idx >= 0 &&
                static_cast<std::size_t>(conn_idx) < peer_socket_index_rev_.size() &&
                chn_idx >= 0 &&
                static_cast<std::size_t>(chn_idx) < peer_socket_index_rev_[conn_idx].size()) {

                std::size_t sock_idx = peer_socket_index_rev_[conn_idx][chn_idx];
                if (sock_idx < peer_sockets_.size() && peer_sockets_[sock_idx]) {
                    // ip is in network byte order (from sockaddr_in.sin_addr.s_addr).
                    // address_v4(ulong) takes host byte order — use ntohl.
                    asio::ip::udp::endpoint dest(
                        asio::ip::address_v4(ntohl(ip)), port);
                    LOG_DEBUG("Send frame to " + dest.address().to_string() + ":" +
                              std::to_string(dest.port()) + " (" +
                              std::to_string(data.size()) + " bytes)");
                    peer_sockets_[sock_idx]->send_to(data, dest);
                    return;
                }
            }
            LOG_WARN("Send frame: invalid (conn=" + std::to_string(conn_idx) +
                     ", ch=" + std::to_string(chn_idx) + ") -- dropped " +
                     std::to_string(data.size()) + " bytes");
        });

    // Start the processing-cycle timer
    cycle_period_ms_ = config_.local_rssp1_params.main_cycle_ms;
    start_cycle_timer();
    LOG_INFO("Cycle timer started (period=" +
             std::to_string(cycle_period_ms_) + "ms)");

    log_config_summary();

    LOG_INFO("Forwarder initialized -- " +
             std::to_string(local_sockets_.size()) +
             " local-app socket(s), " +
             std::to_string(peer_sockets_.size()) +
             " RSSP1 peer socket(s) ready");
}

Forwarder::~Forwarder()
{
    // Members destroy in reverse declaration order:
    // last_peer_rx_times_, local_peers_, peer_sockets_, local_sockets_,
    // tx/rx queues, cycle_timer_, io_, config_
    // Each UdpSocket destructor closes its socket; the timer's
    // pending async_wait gets operation_aborted on destruction.
}

// ============================================================================
// run()
// ============================================================================

int Forwarder::run()
{
    LOG_INFO("Entering I/O event loop (Ctrl+C to stop)");
    io_.run();
    LOG_INFO("I/O event loop exited");
    return 0;
}

// ============================================================================
// Cycle timer
// ============================================================================

void Forwarder::start_cycle_timer()
{
    cycle_timer_.expires_after(std::chrono::milliseconds(cycle_period_ms_));
    cycle_timer_.async_wait([this](std::error_code ec) { on_cycle_tick(ec); });
}

void Forwarder::on_cycle_tick(std::error_code ec)
{
    if (ec == asio::error::operation_aborted) return;

    if (ec) {
        LOG_ERROR("Cycle timer error: " + ec.message());
        start_cycle_timer(); // re-arm despite error
        return;
    }

    rssp1_.vsn_update();    // advance RSSP1 timestamp each cycle
    if (rssp1_to_udp_)  rssp1_to_udp_->process();
    if (udp_to_rssp1_)  udp_to_rssp1_->process();

    // Re-arm for the next cycle
    start_cycle_timer();
}

// ============================================================================
// Socket creation
// ============================================================================

void Forwarder::create_sockets()
{
    // ---- Local-app sockets (one per connection) ----
    for (std::size_t ci = 0; ci < config_.connections.size(); ++ci) {
        const auto& conn = config_.connections[ci];
        try {
            auto local_ep = asio::ip::udp::endpoint(
                asio::ip::make_address(conn.udp_channel.local_ip),
                conn.udp_channel.local_port);
            local_sockets_.push_back(std::make_unique<UdpSocket>(io_, local_ep));
        } catch (const std::exception& e) {
            throw std::runtime_error(
                std::string("Failed to create local-app socket for connection ") +
                std::to_string(ci) + " (" + conn.udp_channel.local_ip + ':' +
                std::to_string(conn.udp_channel.local_port) +
                "): " + e.what());
        }
    }

    // Initialize per-connection peer state arrays
    local_peers_.resize(config_.connections.size());
    last_peer_rx_times_.resize(config_.connections.size());

    // Initialize reverse peer socket mapping table
    peer_socket_index_rev_.resize(config_.connections.size());
    for (std::size_t ci = 0; ci < config_.connections.size(); ++ci) {
        peer_socket_index_rev_[ci].resize(
            config_.connections[ci].rssp1_channels.size(),
            static_cast<std::size_t>(-1));
    }

    // ---- RSSP1 peer channel sockets ----
    for (std::size_t ci = 0; ci < config_.connections.size(); ++ci) {
        const auto& conn = config_.connections[ci];
        for (std::size_t chi = 0; chi < conn.rssp1_channels.size(); ++chi) {
            const auto& ch = conn.rssp1_channels[chi];
            try {
                auto ep = asio::ip::udp::endpoint(
                    asio::ip::make_address(ch.local_ip),
                    ch.local_port);
                auto sock = std::make_unique<UdpSocket>(io_, ep);
                auto flat_idx = peer_sockets_.size();
                peer_sockets_.push_back(std::move(sock));
                peer_socket_conn_idx_.push_back(static_cast<int>(ci));
                peer_socket_ch_idx_.push_back(static_cast<int>(chi));
                peer_socket_index_rev_[ci][chi] = flat_idx;
            } catch (const std::exception& e) {
                throw std::runtime_error(
                    std::string("Failed to create RSSP1 peer socket [conn ") +
                    std::to_string(ci) + ", ch " + std::to_string(chi) +
                    "] (" + ch.local_ip + ':' +
                    std::to_string(ch.local_port) + "): " + e.what());
            }
        }
    }

    if (local_sockets_.empty()) {
        throw std::runtime_error(
            "No local UDP sockets configured -- at least one connection "
            "with a udp_channel is required in connections[]");
    }

    if (peer_sockets_.empty()) {
        throw std::runtime_error(
            "No RSSP1 peer sockets configured -- at least one rssp1_channel "
            "is required in connections[].rssp1_channels");
    }
}

// ============================================================================
// Receive loop setup
// ============================================================================

void Forwarder::start_receive_loops()
{
    // Local-app sockets -- capture connection index for routing
    for (std::size_t i = 0; i < local_sockets_.size(); ++i) {
        local_sockets_[i]->start_receive(
            [this, i](std::vector<std::uint8_t> data,
                       asio::ip::udp::endpoint sender) {
                on_local_app_datagram(static_cast<int>(i),
                                      std::move(data),
                                      std::move(sender));
            });
    }

    // RSSP1 peer sockets -- capture flat index for logging
    for (std::size_t i = 0; i < peer_sockets_.size(); ++i) {
        peer_sockets_[i]->start_receive(
            [this, i](std::vector<std::uint8_t> data,
                       asio::ip::udp::endpoint sender) {
                on_rssp1_datagram(i, std::move(data), std::move(sender));
            });
    }
}

// ============================================================================
// Config summary logging
// ============================================================================

void Forwarder::log_config_summary()
{
    LOG_INFO("Configuration summary:");
    LOG_INFO("  log_level: " +
             std::string(log_level_to_string(config_.log_level)));
    LOG_INFO("  local_rssp1_params.main_cycle_ms: " +
             std::to_string(config_.local_rssp1_params.main_cycle_ms));
    LOG_INFO("  connections: " +
             std::to_string(config_.connections.size()));

    for (std::size_t i = 0; i < config_.connections.size(); ++i) {
        const auto& conn = config_.connections[i];
        std::ostringstream addr_hex;
        addr_hex << "0x" << std::hex << std::uppercase << conn.addr;
        LOG_INFO("    [" + std::to_string(i) + "] dest_addr=" +
                 addr_hex.str() +
                 ", udp_channel=" + conn.udp_channel.local_ip + ':' +
                 std::to_string(conn.udp_channel.local_port) +
                 " (peer_timeout=" +
                 std::to_string(conn.udp_channel.peer_timeout_ms) + "ms)" +
                 ", rssp1_channels=" +
                 std::to_string(conn.rssp1_channels.size()));
        for (std::size_t j = 0; j < conn.rssp1_channels.size(); ++j) {
            const auto& ch = conn.rssp1_channels[j];
            LOG_INFO("        ch[" + std::to_string(j) + "] " +
                     ch.local_ip + ':' + std::to_string(ch.local_port) +
                     " -> " + ch.remote_ip + ':' +
                     std::to_string(ch.remote_port));
        }
    }
}

// ============================================================================
// Local-app datagram handler (auto-learn peer + push to tx queue)
// ============================================================================

void Forwarder::on_local_app_datagram(int conn_idx,
                                       std::vector<std::uint8_t> data,
                                       asio::ip::udp::endpoint sender)
{
    // 1. Check if the current learned peer has timed out
    check_peer_timeout(conn_idx);

    // 2. If no peer is established (fresh start or just pruned), learn sender
    if (!local_peers_[conn_idx].has_value()) {
        learn_peer(conn_idx, sender);

        tx_payload_queue_.push_back({std::move(data), sender, conn_idx});
        LOG_DEBUG("Local-app RX [conn " + std::to_string(conn_idx) +
                  "]: pushed " +
                  std::to_string(tx_payload_queue_.back().data.size()) +
                  " bytes from " + endpoint_to_string(sender) +
                  " to tx_payload_queue (depth=" +
                  std::to_string(tx_payload_queue_.size()) + ")");
        return;
    }

    // 3. Peer is locked in -- only accept from the same address
    if (sender == *local_peers_[conn_idx]) {
        last_peer_rx_times_[conn_idx] = std::chrono::steady_clock::now();

        tx_payload_queue_.push_back({std::move(data), sender, conn_idx});
        LOG_DEBUG("Local-app RX [conn " + std::to_string(conn_idx) +
                  "]: pushed " +
                  std::to_string(tx_payload_queue_.back().data.size()) +
                  " bytes from " + endpoint_to_string(sender) +
                  " to tx_payload_queue (depth=" +
                  std::to_string(tx_payload_queue_.size()) + ")");
    } else {
        // Silently drop datagrams from unknown senders
        LOG_DEBUG("Dropped " + std::to_string(data.size()) +
                  " bytes from unknown sender " +
                  endpoint_to_string(sender) +
                  " on conn " + std::to_string(conn_idx) +
                  " (current peer: " +
                  endpoint_to_string(*local_peers_[conn_idx]) + ')');
    }
}

// ============================================================================
// RSSP1 peer datagram handler (push to rx queue)
// ============================================================================

void Forwarder::on_rssp1_datagram(std::size_t socket_index,
                                   std::vector<std::uint8_t> data,
                                   asio::ip::udp::endpoint sender)
{
    int conn_idx = resolve_socket_to_conn_idx(socket_index);
    rx_frame_queue_.push_back({std::move(data), sender, conn_idx});
    LOG_DEBUG("RSSP1 peer [conn " +
              std::to_string(resolve_socket_to_conn_idx(socket_index)) +
              ", ch " +
              std::to_string(resolve_socket_to_ch_idx(socket_index)) +
              "] RX: pushed " +
              std::to_string(rx_frame_queue_.back().raw_bytes.size()) +
              " bytes from " + endpoint_to_string(sender) +
              " to rx_frame_queue (depth=" +
              std::to_string(rx_frame_queue_.size()) + ")");
}

// ============================================================================
// Auto-learn peer helpers
// ============================================================================

void Forwarder::learn_peer(int conn_idx, const asio::ip::udp::endpoint& sender)
{
    local_peers_[conn_idx] = sender;
    last_peer_rx_times_[conn_idx] = std::chrono::steady_clock::now();
    LOG_INFO("=== Learned local UDP peer for conn " +
             std::to_string(conn_idx) + ": " +
             endpoint_to_string(sender) + " ===");
}

void Forwarder::prune_peer(int conn_idx)
{
    if (!local_peers_[conn_idx].has_value()) return;

    LOG_INFO("Local peer " + endpoint_to_string(*local_peers_[conn_idx]) +
             " on conn " + std::to_string(conn_idx) +
             " timed out -- pruning");
    local_peers_[conn_idx].reset();
}

void Forwarder::check_peer_timeout(int conn_idx)
{
    if (!local_peers_[conn_idx].has_value()) return;

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - last_peer_rx_times_[conn_idx]);

    if (elapsed.count() >= config_.connections[conn_idx].udp_channel.peer_timeout_ms) {
        prune_peer(conn_idx);
    }
}

// ============================================================================
// Send to local peer
// ============================================================================

void Forwarder::send_to_local_peer(int conn_idx,
                                    const std::vector<std::uint8_t>& data)
{
    if (conn_idx < 0 || static_cast<std::size_t>(conn_idx) >= local_sockets_.size()) {
        LOG_WARN("send_to_local_peer: invalid connection index " +
                 std::to_string(conn_idx) + " -- dropping " +
                 std::to_string(data.size()) + " bytes");
        return;
    }

    if (!local_peers_[conn_idx].has_value()) {
        LOG_WARN("send_to_local_peer [conn " + std::to_string(conn_idx) +
                 "]: no peer established -- dropping " +
                 std::to_string(data.size()) + " bytes");
        return;
    }
    local_sockets_[conn_idx]->send_to(data, *local_peers_[conn_idx]);
}

// ============================================================================
// Utility
// ============================================================================

std::string Forwarder::endpoint_to_string(
    const asio::ip::udp::endpoint& ep) const
{
    return ep.address().to_string() + ':' + std::to_string(ep.port());
}

int Forwarder::resolve_socket_to_conn_idx(std::size_t socket_index) const
{
    if (socket_index < peer_socket_conn_idx_.size()) {
        return peer_socket_conn_idx_[socket_index];
    }
    return -1;
}

int Forwarder::resolve_socket_to_ch_idx(std::size_t socket_index) const
{
    if (socket_index < peer_socket_ch_idx_.size()) {
        return peer_socket_ch_idx_[socket_index];
    }
    return -1;
}

} // namespace forwarder
