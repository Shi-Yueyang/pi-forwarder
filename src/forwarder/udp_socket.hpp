#pragma once

// RAII wrapper around asio::ip::udp::socket with an async receive loop
// and fire-and-forget send. Single-threaded -- no strand or locking needed.

#include <asio.hpp>

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace forwarder {

class UdpSocket {
public:
    /// Callback invoked on each received datagram.
    /// Signature: void(std::vector<std::uint8_t> data, asio::ip::udp::endpoint sender)
    using ReceiveCallback =
        std::function<void(std::vector<std::uint8_t>, asio::ip::udp::endpoint)>;

    /// Open and bind the socket to @p local_endpoint.
    /// Throws std::runtime_error on bind failure (address in use, bad IP, etc.).
    explicit UdpSocket(asio::io_context& io,
                       const asio::ip::udp::endpoint& local_endpoint);

    ~UdpSocket();

    // Non-copyable, non-movable.
    // Move is deliberately deleted -- pending async handlers capture `this`,
    // so moving would dangle those captures. Use std::unique_ptr<UdpSocket>
    // in containers instead.
    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;
    UdpSocket(UdpSocket&&) = delete;
    UdpSocket& operator=(UdpSocket&&) = delete;

    /// Start the continuous async receive loop. @p callback is invoked for
    /// every successfully received datagram. The loop re-arms automatically.
    /// Call once after construction; subsequent calls are a no-op.
    void start_receive(ReceiveCallback callback);

    /// Fire-and-forget async send to @p destination.
    /// Data is copied internally so the caller can discard it immediately.
    void send_to(const std::vector<std::uint8_t>& data,
                 const asio::ip::udp::endpoint& destination);

    /// The local endpoint this socket is bound to.
    const asio::ip::udp::endpoint& local_endpoint() const { return local_ep_; }

private:
    void do_receive();
    void on_receive_complete(std::shared_ptr<std::vector<std::uint8_t>> buffer,
                             std::shared_ptr<asio::ip::udp::endpoint> sender,
                             std::error_code ec,
                             std::size_t bytes_transferred);

    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint local_ep_;
    asio::ip::udp::endpoint remote_ep_; // mutable, reused across async receives
    ReceiveCallback on_datagram_;
    bool receiving_{false};
};

} // namespace forwarder
