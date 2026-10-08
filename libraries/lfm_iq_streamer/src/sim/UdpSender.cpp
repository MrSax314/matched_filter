#include "radarsim/sim/UdpSender.hpp"

#include <cstring>
#include <stdexcept>

#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>
using SocketHandle                     = SOCKET;
constexpr SocketHandle kInvalidSocket  = INVALID_SOCKET;
static void            closeSocket(SocketHandle s) { closesocket(s); }
#else
    #include <netdb.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <unistd.h>
using SocketHandle                     = int;
constexpr SocketHandle kInvalidSocket  = -1;
static void            closeSocket(SocketHandle s) { ::close(s); }
#endif

namespace radarsim {

struct UdpSender::Impl {
    SocketHandle     sock = kInvalidSocket;
    sockaddr_storage dest{};
    socklen_t        destLen = 0;
#ifdef _WIN32
    bool wsaStarted = false;
#endif

    // Runs even if the UdpSender constructor throws part-way through.
    ~Impl() {
        if (sock != kInvalidSocket) closeSocket(sock);
#ifdef _WIN32
        if (wsaStarted) WSACleanup();
#endif
    }
};

UdpSender::UdpSender(const std::string& host, uint16_t port) : impl_(std::make_unique<Impl>()) {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) throw std::runtime_error("WSAStartup failed");
    impl_->wsaStarted = true;
#endif

    addrinfo hints{};
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    addrinfo*         result  = nullptr;
    const std::string portStr = std::to_string(port);
    if (getaddrinfo(host.c_str(), portStr.c_str(), &hints, &result) != 0 || result == nullptr)
        throw std::runtime_error("cannot resolve host '" + host + "'");

    impl_->sock = ::socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (impl_->sock == kInvalidSocket) {
        freeaddrinfo(result);
        throw std::runtime_error("cannot create UDP socket");
    }
    std::memcpy(&impl_->dest, result->ai_addr, result->ai_addrlen);
    impl_->destLen = static_cast<socklen_t>(result->ai_addrlen);
    freeaddrinfo(result);

    // A larger send buffer absorbs bursts when the OS sleep granularity is coarse.
    const int sendBuffer = 4 * 1024 * 1024;
    setsockopt(impl_->sock, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const char*>(&sendBuffer),
               sizeof(sendBuffer));
}

UdpSender::~UdpSender() = default;

bool UdpSender::send(const uint8_t* data, std::size_t size) {
    const auto sent = ::sendto(impl_->sock, reinterpret_cast<const char*>(data), static_cast<int>(size), 0,
                               reinterpret_cast<const sockaddr*>(&impl_->dest), impl_->destLen);
    return sent >= 0 && static_cast<std::size_t>(sent) == size;
}

} // namespace radarsim
