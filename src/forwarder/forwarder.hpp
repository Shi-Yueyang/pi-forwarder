#pragma once

// Top-level orchestrator that owns the asio::io_context and coordinates
// UDP I/O with the RSSP1 adapter.

#include "config.hpp"
#include "queues.hpp"
#include "rssp1_adapter.hpp"
#include "udp_socket.hpp"

#include <asio.hpp>

#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace forwarder {

class Forwarder {
public:
    explicit Forwarder(const Config& config);
    ~Forwarder();

    // Non-copyable
    Forwarder(const Forwarder&) = delete;
    Forwarder& operator=(const Forwarder&) = delete;

    /// Start the I/O event loop. Blocks until io_context is stopped.
    /// In Phase 3 this runs until Ctrl+C (no graceful shutdown yet).
    int run();

    /// Send data to the learned local UDP peer.
    /// No-op if no peer is currently established; logs a warning.
    void send_to_local_peer(const std::vector<std::uint8_t>& data);

private:
    // ---- Setup ----
    void create_sockets();
    void start_receive_loops();
    void start_cycle_timer();
    void log_config_summary();

    // ---- Cycle processing ----
    void on_cycle_tick(std::error_code ec);
    void do_receive_pass();
    void do_send_pass();

    // ---- Receive handlers ----
    void on_local_app_datagram(std::vector<std::uint8_t> data,
                               asio::ip::udp::endpoint sender);
    void on_rssp1_datagram(std::size_t socket_index,
                           std::vector<std::uint8_t> data,
                           asio::ip::udp::endpoint sender);

    // ---- Auto-learn peer helpers ----
    void learn_peer(const asio::ip::udp::endpoint& sender);
    void prune_peer();
    void check_peer_timeout();

    // ---- Utility ----
    std::string endpoint_to_string(const asio::ip::udp::endpoint& ep) const;
    int resolve_socket_to_conn_idx(std::size_t socket_index) const;
    int resolve_socket_to_ch_idx(std::size_t socket_index) const;

    // ---- Members (order matters for destructor: ASIO deps before io_context) ----
    Config config_;
    asio::io_context io_;

    // Cycle timer (declared after io_ so it's destroyed before io_)
    asio::steady_timer cycle_timer_{io_};
    int cycle_period_ms_ = 200;

    // Decoupling queues (push from I/O handlers, pop from cycle)
    RxFrameQueue rx_frame_queue_;
    TxPayloadQueue tx_payload_queue_;

    // RSSP1 protocol stack adapter (initialized with generated INI)
    Rssp1Adapter rssp1_;

    // Local-app socket (one per forwarder)
    std::unique_ptr<UdpSocket> local_socket_;

    // RSSP1 peer channel sockets (one per connection x channel)
    // Parallel arrays for mapping flat index -> (conn_idx, ch_idx)
    std::vector<std::unique_ptr<UdpSocket>> peer_sockets_;
    std::vector<int> peer_socket_conn_idx_;
    std::vector<int> peer_socket_ch_idx_;

    // Auto-learn state for the local-app socket
    std::optional<asio::ip::udp::endpoint> local_peer_;
    std::chrono::steady_clock::time_point last_peer_rx_time_;
};

} // namespace forwarder
