#include "udp_to_rssp1.hpp"
#include "log.hpp"

#include <utility>

namespace forwarder {

UdpToRsp1::UdpToRsp1(TxPayloadQueue& tx_queue,
                     Rssp1Adapter& adapter,
                     const Config& config,
                     SendFrameCallback send_frame)
    : tx_queue_(tx_queue)
    , adapter_(adapter)
    , config_(config)
    , send_frame_(std::move(send_frame))
{
}

void UdpToRsp1::process()
{
    // ---- Step 1: Submit queued UDP payloads to the RSSP1 stack ----
    while (!tx_queue_.empty()) {
        auto& payload = tx_queue_.front();

        if (payload.conn_idx >= 0 &&
            static_cast<std::size_t>(payload.conn_idx) < config_.connections.size()) {

            std::uint16_t dest_addr = config_.connections[payload.conn_idx].addr;

            bool submitted = adapter_.send_app_data(payload.data, dest_addr);

            if (submitted) {
                LOG_DEBUG("UDP->RSSP1 [conn " +
                          std::to_string(payload.conn_idx) + "]: " +
                          std::to_string(payload.data.size()) +
                          " bytes -> dest_addr=0x" +
                          std::to_string(dest_addr));
            } else {
                LOG_WARN("UDP->RSSP1 [conn " +
                         std::to_string(payload.conn_idx) + "]: " +
                         "send_app_data failed (dest_addr=0x" +
                         std::to_string(dest_addr) + ") -- dropped");
            }
        } else {
            LOG_WARN("UDP->RSSP1: invalid conn_idx " +
                     std::to_string(payload.conn_idx) + " in tx_queue -- dropped");
        }

        tx_queue_.pop_front();
    }

    // ---- Step 2: Execute the send processing pass ----
    // This must run every cycle even if no app data was submitted.
    // The stack emits periodic protocol frames (SSE, SSR, RSD) that
    // keep the safety connection alive.
    adapter_.send_pass();

    // ---- Step 3: Drain all outbound frames ----
    // The stack may produce one frame per connection per cycle (protocol
    // frames + any app-data frames). Loop until no more frames are pending.
    SendFrame frame;
    while (adapter_.drain_to_send(frame)) {
        LOG_DEBUG("RSSP1 outbound frame [conn " +
                  std::to_string(frame.conn_index) +
                  ", ch " + std::to_string(frame.chn_index) + "]: " +
                  std::to_string(frame.data.size()) + " bytes");

        send_frame_(frame.conn_index, frame.chn_index,
                    std::move(frame.data),
                    frame.ip, frame.port);
    }
}

} // namespace forwarder
