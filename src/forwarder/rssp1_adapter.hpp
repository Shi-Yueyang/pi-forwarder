#pragma once

// C++ wrapper around the GM_RSSP1_* C APIs.
// All extern "C" RSSP1 headers are included only in the .cpp file.
// No other C++ file in the project includes RSSP1 C headers.

#include <cstdint>
#include <string>
#include <vector>

namespace forwarder {

/// A verified application payload received from an RSSP1 peer.
struct ReceivedPayload {
    std::vector<std::uint8_t> data;   // application data (6-byte header already stripped)
    std::uint32_t source_addr;        // SaCEPID of the sending peer
};

/// A raw outbound frame that the RSSP1 stack wants to send over UDP.
struct SendFrame {
    std::vector<std::uint8_t> data;   // raw RSSP1 frame bytes
    std::uint32_t ip;                 // destination IP (network byte order)
    std::uint32_t port;               // destination port (host byte order)
    std::uint16_t conn_index;         // connection index in RSSP1 config
    std::uint8_t  chn_index;          // channel / redundancy index
};

class Rssp1Adapter {
public:
    Rssp1Adapter();
    ~Rssp1Adapter();

    // Non-copyable
    Rssp1Adapter(const Rssp1Adapter&) = delete;
    Rssp1Adapter& operator=(const Rssp1Adapter&) = delete;

    // ---- Initialization ----

    /// Initialize the RSSP1 stack with the generated INI string.
    /// Returns true on success, false on failure.
    bool init(const std::string& ini_content);

    /// Returns true if the stack was successfully initialized.
    bool is_initialized() const { return initialized_; }

    // ---- Data input (called from I/O handlers, outside the cycle) ----

    /// Feed a raw frame received from the network into the RSSP1 stack.
    /// @param frame     raw bytes from the UDP datagram
    /// @param src_ip    sender IP in network byte order (sockaddr_in.sin_addr.s_addr)
    /// @param src_port  sender port in host byte order (ntohs(sockaddr_in.sin_port))
    /// @param mode      mode 1 = auto-index by IP+port
    /// @returns true if the frame matched a configured connection
    bool rcv_com_interface(const std::vector<std::uint8_t>& frame,
                           std::uint32_t src_ip, std::uint32_t src_port,
                           std::uint8_t mode = 1);

    // ---- Cycle processing (called from do_receive_pass / do_send_pass) ----

    /// Execute the RSSP1 receive pass (RxPrc).
    void receive_pass();

    /// Execute the RSSP1 send pass (TxPrc).
    void send_pass();

    /// Drain received application payloads after receive_pass().
    /// Call in a loop until it returns <= 0.
    /// @param out  appended with each payload (6-byte header already stripped)
    /// @returns 1 for app data, 0 for non-app message, -1 for no more
    int drain_received(std::vector<ReceivedPayload>& out);

    /// Submit application data for sending to a specific RSSP1 peer.
    /// Call during send_pass, before drain_to_send().
    /// @returns true if the data was queued successfully
    bool send_app_data(const std::vector<std::uint8_t>& data,
                       std::uint16_t dest_addr);

    /// Drain a single outbound frame after send_pass().
    /// Call once per cycle (the stack emits at most one frame per cycle).
    /// @returns true if a frame is available in @p out
    bool drain_to_send(SendFrame& out);

    // ---- Status / control ----

    /// Set the RSSP1 stack's internal log level.
    /// Levels: 0=off, 1=error, 2=warning, 3=info, 4=debug
    void set_log_level(std::uint16_t level);

    /// Update the VSN timestamp (must be called once per cycle).
    void vsn_update();

private:
    bool initialized_ = false;
};

} // namespace forwarder
