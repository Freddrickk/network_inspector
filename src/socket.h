#ifndef NETWORK_INSPECTOR_SOCKET_H
#define NETWORK_INSPECTOR_SOCKET_H

#include "network_inspector/network_inspector.h"

#include <cstdint>
#include <string>

namespace NetworkInspector
{
    class Socket
    {
    public:
        Socket();
        Socket(uintptr_t id);
        Socket(uintptr_t id, int af, int type);
        ~Socket();

        uintptr_t GetSocketId() const;
        void SetSocketId(uintptr_t id);

        int GetAddressFamily() const;
        void SetAddressFamily(int af);

        std::string GetIPString() const;
        unsigned long GetIPPacked() const;
        void SetIPPacked(unsigned long ip);

        unsigned short GetPort() const;
        unsigned short GetPortPacked() const;

        void SetPortNumber(unsigned short port);
        void SetPortPacked(unsigned short port);

        SocketType GetType() const;
        void SetType(SocketType type);
        void SetType(int type);

        void SetTracking(bool activated);
        bool IsTracked() const;

        static unsigned short GetPortFromNativeSocket(uintptr_t s);
        static SocketType GetTypeFromNativeSocket(uintptr_t s);
        static SocketType GetSocketTypeFromNativeType(int socket_type);
        static std::string GetIPAddressStringFromPacked(unsigned long ip_addr);
        static unsigned short GetPortFromPacked(unsigned short port);

    private:
        uintptr_t m_id;
        int m_af;
        int m_type;
        unsigned long m_ip_addr;
        unsigned short m_port;
        bool m_tracked;
    };
} // namespace NetworkInspector

#endif // NETWORK_INSPECTOR_SOCKET_H
