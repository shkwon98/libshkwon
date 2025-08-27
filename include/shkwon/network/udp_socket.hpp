#pragma once

#include <arpa/inet.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

/**
 * @file udp_socket.hpp
 * @brief Classes for UDP and multicast socket communication.
 */

namespace shkwon::network
{

#ifndef SOCKET
#define SOCKET int
#endif
#ifndef INVALID_SOCKET
#define INVALID_SOCKET ((SOCKET)~0)
#endif

/**
 * @class UdpSocket
 * @brief Class for UDP socket communication.
 *
 * Handles creation, binding, sending, receiving, option setting, and resource cleanup for UDP sockets.
 * - Bind to a specific port/address
 * - Send/receive data
 * - Set multicast interface and receive timeout
 * - Automatic resource cleanup
 */
class UdpSocket
{
public:
    /**
     * @brief Constructor
     * @param[in] port Port to bind (0 for system-assigned)
     * @param[in] addr IP address to bind (empty string for INADDR_ANY)
     */
    UdpSocket(const uint16_t port = 0, const std::string &addr = "");

    /**
     * @brief Destructor (resource cleanup)
     */
    virtual ~UdpSocket();

    /**
     * @brief Create and bind socket
     * @return Bound port on success, -1 on failure
     */
    virtual int Init();

    /**
     * @brief Set multicast interface for outgoing packets
     * @param[in] addr Interface IP address
     * @return true on success, false on failure
     */
    bool SetMulticastInterface(const std::string &addr);

    /**
     * @brief Set multicast interface by name
     * @param[in] if_name Interface name
     * @return true on success, false on failure
     */
    bool SetMulticastInterfaceByName(const std::string &if_name);

    /**
     * @brief Set receive timeout
     * @param[in] timeout Timeout in milliseconds
     * @return true on success, false on failure
     */
    bool SetReceiveTimeout(const std::chrono::milliseconds &timeout);

    /**
     * @brief Set IP_PKTINFO option
     * @param enable Enable or disable the IP_PKTINFO option
     * @return true on success, false on failure
     */
    bool SetIpPacketInfo(bool enable = true);

    /**
     * @brief Bind socket to specific interface for egress traffic
     * @param[in] if_name Interface name
     * @return true on success, false on failure
     */
    bool SetBindToDevice(const std::string &if_name);

    /**
     * @brief Send data to destination
     * @param[in] buf Data buffer to send
     * @param[in] size Data size
     * @param[in] to_ip Destination IP
     * @param[in] to_port Destination port
     * @return Number of bytes sent, -1 on failure
     */
    ssize_t SendTo(const void *buf, const ssize_t size, const std::string &to_ip, const uint16_t to_port) const;

    /**
     * @brief Receive data (returns sender IP/port)
     * @param[out] buf Buffer to store received data
     * @param[in] size Maximum receive size
     * @param[out] from_ip Sender IP
     * @param[out] from_port Sender port
     * @return Number of bytes received, -1 on failure
     */
    ssize_t ReceiveFrom(void *buf, const ssize_t size, std::string &from_ip, uint16_t &from_port);

    /**
     * @brief Receive data (returns sender sockaddr_in)
     * @param[out] buf Buffer to store received data
     * @param[in] size Maximum receive size
     * @param[out] from Sender address structure
     * @return Number of bytes received, -1 on failure
     */
    ssize_t ReceiveFrom(void *buf, const ssize_t size, sockaddr_in &from);

    /**
     * @brief Receives a message
     * @param[out] buf Pointer to an `msghdr` structure where the received message will be stored.
     * @param[in] size The maximum number of bytes to receive.
     * @return The number of bytes received on success, or -1 on failure.
     */
    ssize_t ReceiveMessage(msghdr *buf, const ssize_t size);

    /**
     * @brief Get socket address information
     * @return sockaddr_in structure
     */
    sockaddr_in GetSocketAddress() const;

protected:
    /**
     * @brief Cleanup socket resources
     */
    void Cleanup();

    /**
     * @brief Create UDP socket
     * @return true on success, false on failure
     */
    bool Create();

    /**
     * @brief Set address reuse option
     * @param reuse Enable/disable reuse
     * @return true on success, false on failure
     */
    bool ReuseAddress(bool reuse = true);

    /**
     * @brief Set port reuse option
     * @param reuse Enable/disable reuse
     * @return true on success, false on failure
     */
    bool ReusePort(bool reuse = true);

    /**
     * @brief Bind socket
     * @return Bound port on success, -1 on failure
     */
    int Bind();

    /**
     * @brief Get socket file descriptor
     * @return File descriptor
     */
    SOCKET GetSocketFd() const;

protected:
    SOCKET fd_ = INVALID_SOCKET; ///< Socket file descriptor
    mutable std::mutex mtx_send_;
    mutable std::mutex mtx_recv_;

private:
    sockaddr_in sockaddr_; ///< Socket address structure

    friend class UdpSocketManager;
};

/**
 * @typedef UnicastSocket
 * @brief Type alias for unicast UDP socket
 */
using UnicastSocket = UdpSocket;

/**
 * @class MulticastSocket
 * @brief Class for multicast UDP socket communication.
 *
 * Provides multicast group join/leave, TTL/loopback settings, and related multicast features.
 */
class MulticastSocket : public UdpSocket
{
public:
    /**
     * @brief Constructor
     * @param[in] port Port to bind (0 for system-assigned)
     * @param[in] addr Interface address to bind (empty string for INADDR_ANY)
     */
    MulticastSocket(const uint16_t port = 0, const std::string &addr = "");

    /**
     * @brief Destructor (leave multicast group)
     */
    ~MulticastSocket();

    /**
     * @brief Initialize socket
     * @return Bound port on success, -1 on failure
     */
    int Init() override;

    /**
     * @brief Join multicast group
     * @param[in] mcast_addr Multicast address
     * @param[in] iface_name Interface name (empty for default)
     * @return true on success, false on failure
     */
    bool JoinMulticastGroup(const std::string &mcast_addr, const std::string &iface_name = "");

    /**
     * @brief Leave multicast group
     */
    void LeaveMulticastGroup();

    /**
     * @brief Set multicast loopback
     * @param loopback Enable/disable loopback
     * @return true on success, false on failure
     */
    bool SetLoopback(bool loopback);

    /**
     * @brief Set multicast TTL (Time-to-live)
     * @param ttl TTL value (1~255)
     * @return true on success, false on failure
     */
    bool SetTTL(int ttl = 3);

private:
    std::vector<ip_mreqn> mreq_list_; ///< List of joined multicast groups
};

inline UdpSocket::UdpSocket(const uint16_t port, const std::string &addr)
{
    sockaddr_.sin_family = AF_INET;
    sockaddr_.sin_port = htons(port);
    sockaddr_.sin_addr.s_addr = addr.empty() ? htonl(INADDR_ANY) : inet_addr(addr.c_str());
}

inline UdpSocket::~UdpSocket()
{
    Cleanup();
}

inline int UdpSocket::Init()
{
    Cleanup();

    if (!Create() || !ReuseAddress(true) || !ReusePort(true))
    {
        Cleanup();
        return -1;
    }

    int bound_port = Bind();
    if (bound_port < 0)
    {
        Cleanup();
        return -1;
    }

    return bound_port;
}

inline bool UdpSocket::SetMulticastInterface(const std::string &addr)
{
    std::scoped_lock lock(mtx_recv_, mtx_send_);

    if (fd_ == INVALID_SOCKET)
    {
        return false;
    }

    auto if_addr = in_addr{ .s_addr = inet_addr(addr.c_str()) };
    return setsockopt(fd_, IPPROTO_IP, IP_MULTICAST_IF, &if_addr, sizeof(if_addr)) == 0;
}

inline bool UdpSocket::SetMulticastInterfaceByName(const std::string &if_name)
{
    std::scoped_lock lock(mtx_recv_, mtx_send_);

    if (fd_ == INVALID_SOCKET || if_name.empty())
    {
        return false;
    }

    in_addr found_addr{};
    found_addr.s_addr = htonl(INADDR_ANY);

    // Resolve IPv4 address of the specified interface
    ifaddrs *ifaddr = nullptr;
    if (getifaddrs(&ifaddr) == 0)
    {
        for (auto *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
        {
            if (!ifa || !ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET)
            {
                continue;
            }

            if (if_name == ifa->ifa_name)
            {
                found_addr = reinterpret_cast<sockaddr_in *>(ifa->ifa_addr)->sin_addr;
                break;
            }
        }

        freeifaddrs(ifaddr);
    }

    if (found_addr.s_addr == htonl(INADDR_ANY))
    {
        return false;
    }

    return setsockopt(fd_, IPPROTO_IP, IP_MULTICAST_IF, &found_addr, sizeof(found_addr));
}

inline bool UdpSocket::SetReceiveTimeout(const std::chrono::milliseconds &timeout)
{
    std::scoped_lock lock(mtx_recv_, mtx_send_);

    if (fd_ == INVALID_SOCKET)
    {
        return false;
    }

    auto tv = timeval{ .tv_sec = timeout.count() / 1000, .tv_usec = (timeout.count() % 1000) * 1000 };
    return setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == 0;
}

inline bool UdpSocket::SetIpPacketInfo(bool enable)
{
    std::scoped_lock lock(mtx_recv_, mtx_send_);

    if (fd_ == INVALID_SOCKET)
    {
        return false;
    }

    int optval = enable ? 1 : 0;
    return setsockopt(fd_, IPPROTO_IP, IP_PKTINFO, &optval, sizeof(optval)) == 0;
}

inline bool UdpSocket::SetBindToDevice(const std::string &if_name)
{
    std::scoped_lock lock(mtx_recv_, mtx_send_);

    if (fd_ == INVALID_SOCKET)
    {
        return false;
    }

    // Note: requires CAP_NET_ADMIN
    return setsockopt(fd_, SOL_SOCKET, SO_BINDTODEVICE, if_name.c_str(), if_name.size()) == 0;
}

inline ssize_t UdpSocket::SendTo(const void *buf, const ssize_t size, const std::string &to_ip, const uint16_t to_port) const
{
    sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(to_port);
    dest_addr.sin_addr.s_addr = inet_addr(to_ip.c_str());

    std::scoped_lock lock(mtx_send_);

    if (fd_ == INVALID_SOCKET)
    {
        return -1;
    }

    return sendto(fd_, buf, size, 0, reinterpret_cast<sockaddr *>(&dest_addr), sizeof(dest_addr));
}

inline ssize_t UdpSocket::ReceiveFrom(void *buf, const ssize_t size, std::string &from_ip, uint16_t &from_port)
{
    ssize_t received_bytes;
    sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);

    {
        std::scoped_lock lock(mtx_recv_);

        if (fd_ == INVALID_SOCKET)
        {
            return -1;
        }

        received_bytes = recvfrom(fd_, buf, size, 0, reinterpret_cast<sockaddr *>(&src_addr), &addr_len);
    }

    if (received_bytes < 0)
    {
        return -1;
    }

    from_ip = inet_ntoa(src_addr.sin_addr);
    from_port = ntohs(src_addr.sin_port);
    return received_bytes;
}

inline ssize_t UdpSocket::ReceiveFrom(void *buf, const ssize_t size, sockaddr_in &from)
{
    ssize_t received_bytes;
    socklen_t addr_len = sizeof(from);

    {
        std::scoped_lock lock(mtx_recv_);

        if (fd_ == INVALID_SOCKET)
        {
            return -1;
        }

        received_bytes = recvfrom(fd_, buf, size, 0, reinterpret_cast<sockaddr *>(&from), &addr_len);
    }

    if (received_bytes < 0)
    {
        return -1;
    }

    return received_bytes;
}

inline ssize_t UdpSocket::ReceiveMessage(msghdr *buf, const ssize_t size)
{
    ssize_t received_bytes;

    {
        std::scoped_lock lock(mtx_recv_);

        if (fd_ == INVALID_SOCKET)
        {
            return -1;
        }

        received_bytes = recvmsg(fd_, buf, 0);
    }

    if (received_bytes < 0)
    {
        return -1;
    }

    return received_bytes;
}

inline sockaddr_in UdpSocket::GetSocketAddress() const
{
    return sockaddr_;
}

inline void UdpSocket::Cleanup()
{
    std::scoped_lock lock(mtx_recv_, mtx_send_);

    if (fd_ >= 0)
    {
        close(fd_);
        fd_ = INVALID_SOCKET;
    }
}

inline bool UdpSocket::Create()
{
    std::scoped_lock lock(mtx_recv_, mtx_send_);

    fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    return fd_ != INVALID_SOCKET;
}

inline bool UdpSocket::ReuseAddress(bool reuse)
{
    std::scoped_lock lock(mtx_recv_, mtx_send_);

    if (fd_ == INVALID_SOCKET)
    {
        return false;
    }

    int reuse_addr = reuse ? 1 : 0;
    return setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) == 0;
}

inline bool UdpSocket::ReusePort(bool reuse)
{
    std::scoped_lock lock(mtx_recv_, mtx_send_);

    if (fd_ == INVALID_SOCKET)
    {
        return false;
    }

    int reuse_port = reuse ? 1 : 0;
    return setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &reuse_port, sizeof(reuse_port)) == 0;
}

inline int UdpSocket::Bind()
{
    std::scoped_lock lock(mtx_recv_, mtx_send_);

    if (fd_ == INVALID_SOCKET)
    {
        return -1;
    }

    if (bind(fd_, reinterpret_cast<sockaddr *>(&sockaddr_), sizeof(sockaddr_)) != 0)
    {
        return -1;
    }

    // Discover the port number assigned to the socket
    socklen_t addr_len = sizeof(sockaddr_);
    if (getsockname(fd_, reinterpret_cast<sockaddr *>(&sockaddr_), &addr_len) < 0)
    {
        return -1;
    }

    return ntohs(sockaddr_.sin_port);
}

inline SOCKET UdpSocket::GetSocketFd() const
{
    std::scoped_lock lock(mtx_recv_, mtx_send_);
    return fd_;
}

inline MulticastSocket::MulticastSocket(const uint16_t port, const std::string &addr)
    : UdpSocket(port, addr)
{
}
inline MulticastSocket::~MulticastSocket()
{
    if (GetSocketFd() != INVALID_SOCKET)
    {
        LeaveMulticastGroup();
    }
}

inline int MulticastSocket::Init()
{
    Cleanup();

    if (!Create() || !ReuseAddress(true) || !ReusePort(true) || !SetLoopback(false) || !SetTTL(3))
    {
        Cleanup();
        return -1;
    }

    int bound_port = Bind();
    if (bound_port < 0)
    {
        Cleanup();
        return -1;
    }

    return bound_port;
}

inline bool MulticastSocket::JoinMulticastGroup(const std::string &mcast_addr, const std::string &iface_name)
{
    std::scoped_lock lock(mtx_recv_, mtx_send_);

    if (fd_ == INVALID_SOCKET)
    {
        return false;
    }

    int iface_index = 0;
    if (!iface_name.empty())
    {
        iface_index = if_nametoindex(iface_name.c_str());
        if (iface_index == 0)
        {
            return false;
        }
    }

    ip_mreqn mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(mcast_addr.c_str());
    mreq.imr_address.s_addr = htonl(INADDR_ANY);
    mreq.imr_ifindex = iface_index;

    mreq_list_.push_back(mreq);

    return setsockopt(fd_, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == 0;
}

inline void MulticastSocket::LeaveMulticastGroup()
{
    std::scoped_lock lock(mtx_recv_, mtx_send_);

    for (const auto &mreq : mreq_list_)
    {
        setsockopt(fd_, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
    }
    mreq_list_.clear();
}

inline bool MulticastSocket::SetLoopback(bool loopback)
{
    std::scoped_lock lock(mtx_recv_, mtx_send_);

    if (fd_ == INVALID_SOCKET)
    {
        return false;
    }

    std::uint8_t loopback_val = loopback ? 1 : 0;
    return setsockopt(fd_, IPPROTO_IP, IP_MULTICAST_LOOP, &loopback_val, sizeof(loopback_val)) == 0;
}

inline bool MulticastSocket::SetTTL(int ttl)
{
    std::scoped_lock lock(mtx_recv_, mtx_send_);

    if (fd_ == INVALID_SOCKET)
    {
        return false;
    }

    ttl = std::clamp(ttl, 1, 255);
    return setsockopt(fd_, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) == 0;
}

} // namespace shkwon::network