#pragma once

// UDP payload -> RSSP1 send path.
// Drains the tx_payload_queue, submits application data to the RSSP1 stack,
// runs the send processing pass, and drains outbound RSSP1 frames for
// transmission to the peer.

#include "config.hpp"
#include "queues.hpp"
#include "rssp1_adapter.hpp"

#include <cstdint>
#include <functional>
#include <vector>

namespace forwarder {

class UdpToRsp1 {
public:
    /// Callback invoked for each outbound RSSP1 frame drained from the stack.
    /// @param conn_idx  RSSP1 connection index (from SendFrame.conn_index)
    /// @param chn_idx   channel / redundancy index
    /// @param data      raw frame bytes to send
    /// @param ip        destination IP in network byte order
    /// @param port      destination port in host byte order
    using SendFrameCallback =
        std::function<void(int conn_idx, int chn_idx,
                           std::vector<std::uint8_t> data,
                           std::uint32_t ip, std::uint16_t port)>;

    UdpToRsp1(TxPayloadQueue& tx_queue,
              Rssp1Adapter& adapter,
              const Config& config,
              SendFrameCallback send_frame);

    /// Execute the send pass. Must be called once per cycle, even if the
    /// tx queue is empty (protocol frames SSE/SSR/RSD must be sent).
    void process();

private:
    TxPayloadQueue& tx_queue_;
    Rssp1Adapter& adapter_;
    const Config& config_;
    SendFrameCallback send_frame_;
};

} // namespace forwarder
