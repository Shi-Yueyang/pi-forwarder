#include "rssp1_to_udp.hpp"
#include "log.hpp"

#include <utility>

namespace forwarder {

Rssp1ToUdp::Rssp1ToUdp(RxFrameQueue& rx_queue,
                         Rssp1Adapter& adapter,
                         const Config& config,
                         SendToLocalCallback send_to_local)
    : rx_queue_(rx_queue)
    , adapter_(adapter)
    , config_(config)
    , send_to_local_(std::move(send_to_local))
{
}

void Rssp1ToUdp::process()
{
    // ---- Step 1: Feed queued raw frames into the RSSP1 stack ----
    while (!rx_queue_.empty()) {
        auto& frame = rx_queue_.front();

        // Extract src_ip (network byte order) and src_port (host byte order)
        // from the UDP sender endpoint.
        // to_v4().to_uint() returns the address in network byte order,
        // matching what GM_RSSP1_RCV_com_Interface expects.
        std::uint32_t src_ip   = frame.sender.address().to_v4().to_uint();
        std::uint16_t src_port = frame.sender.port();

        bool matched = adapter_.rcv_com_interface(
            frame.raw_bytes, src_ip, src_port, 1);

        if (!matched) {
            LOG_WARN("RSSP1 frame from " +
                     frame.sender.address().to_string() + ':' +
                     std::to_string(src_port) +
                     " did not match any configured connection -- dropped");
        }

        rx_queue_.pop_front();
    }

    // ---- Step 2: Execute the receive processing pass ----
    // This advances the RSSP1 stack's internal state machines (CFM + SFM).
    // Must be called every cycle even when the queue was empty.
    adapter_.receive_pass();

    // ---- Step 3: Drain all verified application payloads ----
    // drain_received returns payloads with the 6-byte RSSP1 header already
    // stripped. Returns > 0 for app data, 0 for non-app messages, < 0 when
    // no more messages are pending.
    std::vector<ReceivedPayload> payloads;
    int rc;
    while ((rc = adapter_.drain_received(payloads)) > 0) {
        for (auto& payload : payloads) {
            int conn_idx = resolve_conn_idx(payload.source_addr);
            if (conn_idx >= 0) {
                LOG_DEBUG("RSSP1->UDP: " +
                          std::to_string(payload.data.size()) +
                          " bytes from source_addr=0x" +
                          std::to_string(payload.source_addr) +
                          " -> conn " + std::to_string(conn_idx));
                send_to_local_(conn_idx, std::move(payload.data));
            } else {
                LOG_WARN("Received payload from unknown source_addr=0x" +
                         std::to_string(payload.source_addr) + " -- dropped");
            }
        }
        payloads.clear();
    }

    if (rc == 0) {
        LOG_DEBUG("RSSP1 receive_pass produced a non-application message");
    }
}

int Rssp1ToUdp::resolve_conn_idx(std::uint16_t source_addr) const
{
    for (std::size_t i = 0; i < config_.connections.size(); ++i) {
        if (config_.connections[i].addr == source_addr) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

} // namespace forwarder
