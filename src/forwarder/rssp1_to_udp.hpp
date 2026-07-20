#pragma once

// RSSP1 received payload -> UDP send path.
// Drains the rx_frame_queue, feeds frames into the RSSP1 stack, runs the
// receive processing pass, and delivers extracted application payloads
// to the local UDP peer.

#include "config.hpp"
#include "queues.hpp"
#include "rssp1_adapter.hpp"

#include <cstdint>
#include <functional>
#include <vector>

namespace forwarder {

class Rssp1ToUdp {
public:
    /// Callback invoked for each verified application payload.
    /// @param conn_idx  the connection the payload belongs to
    /// @param data      application data (6-byte RSSP1 header already stripped)
    using SendToLocalCallback =
        std::function<void(int conn_idx, std::vector<std::uint8_t> data)>;

    Rssp1ToUdp(RxFrameQueue& rx_queue,
               Rssp1Adapter& adapter,
               const Config& config,
               SendToLocalCallback send_to_local);

    /// Execute the receive pass. Must be called once per cycle, even if the
    /// rx queue is empty (the stack's internal timers must advance).
    void process();

private:
    int resolve_conn_idx(std::uint16_t source_addr) const;

    RxFrameQueue& rx_queue_;
    Rssp1Adapter& adapter_;
    const Config& config_;
    SendToLocalCallback send_to_local_;
};

} // namespace forwarder
