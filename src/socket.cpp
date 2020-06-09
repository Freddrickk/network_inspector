#include "socket.h"

#include <Windows.h>

namespace NetworkInspector
{
    Socket::Socket(): Socket(0, 0, 0)
    {
    }

    Socket::Socket(uintptr_t id): Socket(id, 0, 0)
    {
    }

    Socket::Socket(uintptr_t id, int af, int type): m_id(id), m_af(af), m_type(type), m_ip_addr(0), m_port(0), m_tracked(true)
    {
    }

    Socket::~Socket()
    {
    }

    uintptr_t Socket::GetSocketId() const
    {
        return m_id;
    }

    void Socket::SetSocketId(uintptr_t id)
    {
        m_id = id;
    }

    int Socket::GetAddressFamily() const
    {
        return m_af;
    }

    void Socket::SetAddressFamily(int af)
    {
        m_af = af;
    }

    std::string Socket::GetIPString() const
    {
        return GetIPAddressStringFromPacked(m_ip_addr);
    }

    unsigned long Socket::GetIPPacked() const
    {
        return m_ip_addr;
    }

    void Socket::SetIPPacked(unsigned long ip)
    {
        m_ip_addr = ip;
    }

    unsigned short Socket::GetPort() const
    {
        return GetPortFromPacked(m_port);
    }

    unsigned short Socket::GetPortPacked() const
    {
        return m_port;
    }

    void Socket::SetPortNumber(unsigned short port)
    {
        m_port = htons(port);
    }

    void Socket::SetPortPacked(unsigned short port)
    {
        m_port = port;
    }

    SocketType Socket::GetType() const
    {
        return GetSocketTypeFromNativeType(m_type);
    }

    void Socket::SetType(SocketType type)
    {
        int sock_type = 0;
        if (type == SocketType::SocketTCP)
        {
            sock_type = SOCK_STREAM;
        }
        else if (type == SocketType::SocketUDP)
        {
            sock_type = SOCK_DGRAM;
        }
        m_type = sock_type;
    }

    void Socket::SetType(int type)
    {
        m_type = type;
    }

    void Socket::SetTracking(bool activated)
    {
        m_tracked = activated;
    }

    bool Socket::IsTracked() const
    {
        return m_tracked;
    }

    unsigned short Socket::GetPortFromNativeSocket(uintptr_t s)
    {
        sockaddr_in lsockaddr;
        int lsocklen = sizeof(lsockaddr);
        getsockname(s, (sockaddr*)&lsockaddr, &lsocklen);
        return GetPortFromPacked(lsockaddr.sin_port);
    }

    SocketType Socket::GetTypeFromNativeSocket(uintptr_t s)
    {
        int type = 0;
        int len  = sizeof(int);
        getsockopt(s, SOL_SOCKET, SO_TYPE, (char*)&type, &len);
        return GetSocketTypeFromNativeType(type);
    }

    SocketType Socket::GetSocketTypeFromNativeType(int socket_type)
    {
        SocketType sock_type = SocketType::SocketUnknown;
        if (socket_type == SOCK_STREAM)
        {
            sock_type = SocketType::SocketTCP;
        }
        else if (socket_type == SOCK_DGRAM)
        {
            sock_type = SocketType::SocketUDP;
        }

        return sock_type;
    }

    std::string Socket::GetIPAddressStringFromPacked(unsigned long ip_addr)
    {
        char str[0x10] = {};
        snprintf(str,
                 0x10,
                 "%hhu.%hhu.%hhu.%hhu",
                 (unsigned char)((ip_addr & 0x000000ff) >> 0x00),
                 (unsigned char)((ip_addr & 0x0000ff00) >> 0x08),
                 (unsigned char)((ip_addr & 0x00ff0000) >> 0x10),
                 (unsigned char)((ip_addr & 0xff000000) >> 0x18));
        return std::string(str);
    }

    unsigned short Socket::GetPortFromPacked(unsigned short port)
    {
        return ntohs(port);
    }

} // namespace NetworkInspector
