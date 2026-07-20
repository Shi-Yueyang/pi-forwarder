#pragma once

// Decoupling queues between async UDP I/O and the RSSP1 processing cycle.
// Plain std::deque, no locking -- the whole program is single-threaded.

#include <asio.hpp>

#include <cstdint>
#include <deque>
#include <vector>

namespace forwarder {

/// A raw RSSP1 frame received from a peer UDP channel.
/// Phases 4/6 will feed this into GM_RSSP1_RCV_com_Interface().
struct RxFrame {
    std::vector<std::uint8_t> raw_bytes;
    asio::ip::udp::endpoint sender;   // which RSSP1 peer sent this
    int conn_idx = -1;                // which connection this frame belongs to
};

/// A payload from a local application, waiting to be forwarded to RSSP1.
/// Phases 5/6 will resolve sender -> RSSP1 dest_addr and call Send_App_Dat().
struct TxPayload {
    std::vector<std::uint8_t> data;
    asio::ip::udp::endpoint sender;   // which local app sent this
    int conn_idx = -1;                // which connection this payload is for
};

using RxFrameQueue  = std::deque<RxFrame>;
using TxPayloadQueue = std::deque<TxPayload>;

} // namespace forwarder
