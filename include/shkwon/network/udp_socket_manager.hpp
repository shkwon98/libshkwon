#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "shkwon/network/udp_socket.hpp"

namespace shkwon::network
{

/**
 * @brief Manager class for UDP sockets (managed by file descriptor as key).
 *
 * Provides creation, destruction, readiness polling (select), and lookup
 * of unicast and multicast UDP sockets. Each managed socket is stored
 * using its file descriptor as the map key.
 */
class UdpSocketManager
{
public:
    UdpSocketManager();
    ~UdpSocketManager() = default;

    /**
     * @brief Configure select timeout.
     * @param[in] timeout_ms Timeout in milliseconds; negative value enables blocking (no timeout).
     * @return Reference to this manager (fluent interface).
     */
    UdpSocketManager &SetTimeout(int timeout_ms);

    /**
     * @brief Retrieve currently configured timeout.
     * @return Timeout in milliseconds; -1 if blocking (no timeout set).
     */
    int GetTimeout() const;

    /**
     * @brief Create a unicast UDP socket and register it.
     * @param[in] port Port to bind (0 lets the system choose an ephemeral port).
     * @return File descriptor on success; -1 on failure.
     */
    SOCKET CreateSocket(uint16_t port = 0);

    /**
     * @brief Create a multicast UDP socket, join group, and register it.
     * @param[in] port Port to bind (0 allowed; system assigns).
     * @param[in] group Multicast group IPv4 address (e.g., "239.x.x.x").
     * @param[in] iface_name Optional interface name to join through (empty = default).
     * @return File descriptor on success; -1 on failure.
     */
    SOCKET CreateSocket(const uint16_t port, const std::string &group, const std::string &iface_name = "");

    /**
     * @brief Destroy (unregister and close) a managed socket by its fd.
     * @param[in] fd File descriptor of the target socket.
     * @return true if the socket existed and was removed; false otherwise.
     */
    bool DestroySocket(SOCKET fd);

    /**
     * @brief Poll all registered sockets for readability using select().
     * Copies the select result into the internal fd_set used by GetIfReadable().
     * @return true if select succeeded (or was interrupted by signal EINTR); false if unrecoverable error occurred.
     */
    bool UpdateSocketStatus();

    /**
     * @brief Return pointer to socket if it is marked readable by the last UpdateSocketStatus().
     * @param[in] fd File descriptor to query.
     * @return Pointer to UdpSocket if readable; nullptr otherwise.
     */
    UdpSocket *GetIfReadable(SOCKET fd);

    /**
     * @brief Number of currently managed sockets.
     */
    std::size_t Size();

    /**
     * @brief Remove all sockets and reset internal state.
     */
    void Clear();

private:
    /**
     * @brief Find a managed socket by file descriptor.
     * @param[in] fd File descriptor.
     * @return Pointer if found; nullptr otherwise.
     */
    UdpSocket *FindSocket(SOCKET fd);

    /**
     * @brief Recreate a socket after failure (closes and re-initializes).
     *        The file descriptor may change; the map key is updated accordingly.
     * @param[in] fd Original file descriptor.
     * @return true on successful reinitialization; false on failure (socket removed).
     */
    bool RenewSocket(SOCKET fd);

    fd_set fds_;                                                ///< Readable result set from last select().
    std::unique_ptr<timeval> timeout_;                          ///< Timeout used for select(); nullptr means blocking.
    std::map<SOCKET, std::unique_ptr<UdpSocket>> socket_table_; ///< Managed sockets keyed by fd.
};

inline UdpSocketManager::UdpSocketManager()
    : timeout_{ nullptr }
{
    FD_ZERO(&fds_);
}

inline UdpSocketManager &UdpSocketManager::SetTimeout(int timeout_ms)
{
    if (timeout_ms < 0)
    {
        timeout_.reset();
        return *this;
    }

    auto tv = std::make_unique<timeval>();
    tv->tv_sec = timeout_ms / 1000;
    tv->tv_usec = (timeout_ms % 1000) * 1000;

    timeout_ = std::move(tv);
    return *this;
}

inline int UdpSocketManager::GetTimeout() const
{
    if (!timeout_)
    {
        return -1;
    }

    return timeout_->tv_sec * 1000 + timeout_->tv_usec / 1000;
}

inline SOCKET UdpSocketManager::CreateSocket(uint16_t port)
{
    auto socket = std::make_unique<UdpSocket>(port);
    int bound_port = socket->Init();
    if (bound_port < 0)
    {
        return -1;
    }

    SOCKET fd = socket->GetSocketFd();
    FD_SET(fd, &fds_);
    socket_table_[fd] = std::move(socket);
    return fd;
}

inline SOCKET UdpSocketManager::CreateSocket(const uint16_t port, const std::string &group, const std::string &iface_name)
{
    auto socket = std::make_unique<MulticastSocket>(port);
    int bound_port = socket->Init();
    if (bound_port < 0)
    {
        return -1;
    }

    if (!socket->JoinMulticastGroup(group, iface_name))
    {
        return -1;
    }

    SOCKET fd = socket->GetSocketFd();
    FD_SET(fd, &fds_);
    socket_table_[fd] = std::move(socket);
    return fd;
}

inline bool UdpSocketManager::DestroySocket(SOCKET fd)
{
    auto it = socket_table_.find(fd);
    if (it == socket_table_.end())
    {
        return false;
    }

    FD_CLR(fd, &fds_);
    socket_table_.erase(it);
    return true;
}

inline bool UdpSocketManager::UpdateSocketStatus()
{
    fd_set fds;
    FD_ZERO(&fds);

    // Rebuild the candidate read set from currently valid descriptors.
    SOCKET max_fd = 0;
    for (const auto &[fd, socket] : socket_table_)
    {
        if (fd != INVALID_SOCKET)
        {
            FD_SET(fd, &fds);
            max_fd = std::max(max_fd, fd);
        }
    }

    // Clone timeout because select() can modify it.
    std::unique_ptr<timeval> timeout_tmp;
    if (timeout_)
    {
        timeout_tmp = std::make_unique<timeval>(*timeout_);
    }

    if (select(max_fd + 1, &fds, nullptr, nullptr, timeout_tmp.get()) < 0)
    {
        if (errno != EINTR)
        {
            // On unrecoverable error attempt to renew all sockets.
            std::vector<SOCKET> fds_to_renew;
            fds_to_renew.reserve(socket_table_.size());
            for (const auto &[fd, socket] : socket_table_)
            {
                fds_to_renew.push_back(fd);
            }

            for (auto old_fd : fds_to_renew)
            {
                RenewSocket(old_fd);
            }
            return false;
        }
    }

    // Persist readability results.
    fds_ = fds;
    return true;
}

inline UdpSocket *UdpSocketManager::GetIfReadable(SOCKET fd)
{
    auto socket = FindSocket(fd);
    if (!socket)
    {
        return nullptr;
    }
    SOCKET cur_fd = socket->GetSocketFd();
    if (cur_fd == INVALID_SOCKET)
    {
        return nullptr;
    }

    return FD_ISSET(cur_fd, &fds_) ? socket : nullptr;
}

inline UdpSocket *UdpSocketManager::FindSocket(SOCKET fd)
{
    auto it = socket_table_.find(fd);
    if (it == socket_table_.end())
    {
        return nullptr;
    }

    return it->second.get();
}

inline bool UdpSocketManager::RenewSocket(SOCKET fd)
{
    // Attempts to reinitialize a single socket; fd may change after Init().
    auto it = socket_table_.find(fd);
    if (it == socket_table_.end())
    {
        return false;
    }

    FD_CLR(fd, &fds_);

    auto sock_ptr = std::move(it->second);
    socket_table_.erase(it);

    if (sock_ptr->Init() < 0)
    {
        // Initialization failed: socket will be discarded
        return false;
    }

    SOCKET new_fd = sock_ptr->GetSocketFd();
    if (new_fd == INVALID_SOCKET)
    {
        // Failed to reinitialize socket: invalid file descriptor
        return false;
    }

    FD_SET(new_fd, &fds_);
    socket_table_[new_fd] = std::move(sock_ptr);
    return true;
}

inline std::size_t UdpSocketManager::Size()
{
    return socket_table_.size();
}

inline void UdpSocketManager::Clear()
{
    // Remove all tracked sockets and reset state.
    FD_ZERO(&fds_);
    socket_table_.clear();
}

} // namespace shkwon::network