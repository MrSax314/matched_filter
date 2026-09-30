#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace radarsim {

/// Minimal cross-platform (POSIX / Winsock) UDP sender. Platform headers stay in the .cpp.
class UdpSender {
public:
    /// Resolves host (IPv4/IPv6 address or hostname). Throws std::runtime_error on failure.
    UdpSender(const std::string& host, uint16_t port);
    ~UdpSender();

    UdpSender(const UdpSender&)            = delete;
    UdpSender& operator=(const UdpSender&) = delete;

    /// Sends one datagram. Returns false on a (usually transient) send error.
    bool send(const uint8_t* data, std::size_t size);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace radarsim
