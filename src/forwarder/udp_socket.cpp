#include "udp_socket.hpp"
#include "log.hpp"

#include <stdexcept>
#include <system_error>
#include <utility>

namespace forwarder {

namespace {
    constexpr std::size_t k_max_datagram_size = 65536; // max UDP datagram (64 KiB)
} // anonymous namespace

UdpSocket::UdpSocket(asio::io_context& io,
                     const asio::ip::udp::endpoint& local_endpoint)
    : socket_(io, local_endpoint.protocol())
    , local_ep_(local_endpoint)
{
    std::error_code ec;
    socket_.set_option(asio::ip::udp::socket::reuse_address(true), ec);
    if (ec) {
        throw std::runtime_error(
            "Failed to set reuse_address on " +
            local_endpoint.address().to_string() + ':' +
            std::to_string(local_endpoint.port()) + " -- " + ec.message());
    }

    socket_.bind(local_endpoint, ec);
    if (ec) {
        throw std::runtime_error(
            "Failed to bind UDP socket to " +
            local_endpoint.address().to_string() + ':' +
            std::to_string(local_endpoint.port()) + " -- " + ec.message());
    }

    LOG_INFO("UdpSocket bound to " +
             local_endpoint.address().to_string() + ':' +
             std::to_string(local_endpoint.port()));
}

UdpSocket::~UdpSocket()
{
    std::error_code ec;
    socket_.close(ec);
    // ec ignored -- we're shutting down, nothing to do about errors
}

void UdpSocket::start_receive(ReceiveCallback callback)
{
    if (receiving_) {
        LOG_WARN("UdpSocket::start_receive called more than once -- ignored");
        return;
    }
    on_datagram_ = std::move(callback);
    receiving_ = true;
    do_receive();
}

void UdpSocket::do_receive()
{
    if (!receiving_) return;

    auto buffer = std::make_shared<std::vector<std::uint8_t>>(k_max_datagram_size);
    auto sender = std::make_shared<asio::ip::udp::endpoint>();

    socket_.async_receive_from(
        asio::buffer(*buffer), *sender,
        [this, buffer, sender](std::error_code ec, std::size_t bytes) {
            on_receive_complete(buffer, sender, ec, bytes);
        });
}

void UdpSocket::on_receive_complete(
    std::shared_ptr<std::vector<std::uint8_t>> buffer,
    std::shared_ptr<asio::ip::udp::endpoint> sender,
    std::error_code ec,
    std::size_t bytes_transferred)
{
    if (ec == asio::error::operation_aborted) {
        return; // socket closed, object being destroyed -- stop the loop
    }

    if (ec) {
        LOG_ERROR("UDP receive error on " +
                  local_ep_.address().to_string() + ':' +
                  std::to_string(local_ep_.port()) + " -- " + ec.message());
        do_receive(); // re-arm despite transient error
        return;
    }

    if (on_datagram_) {
        buffer->resize(bytes_transferred);
        on_datagram_(std::move(*buffer), std::move(*sender));
    }

    do_receive(); // re-arm for next datagram
}

void UdpSocket::send_to(const std::vector<std::uint8_t>& data,
                         const asio::ip::udp::endpoint& destination)
{
    // Copy data into a shared_ptr so it outlives the async operation
    auto shared_data = std::make_shared<std::vector<std::uint8_t>>(data);

    socket_.async_send_to(
        asio::buffer(*shared_data), destination,
        [shared_data](std::error_code ec, std::size_t /*bytes_sent*/) {
            if (ec && ec != asio::error::operation_aborted) {
                LOG_ERROR("UDP send_to error: " + ec.message());
            }
        });
}

} // namespace forwarder
