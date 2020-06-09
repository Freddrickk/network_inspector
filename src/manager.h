#ifndef NETWORK_INSPECTOR_MANAGER_H_
#define NETWORK_INSPECTOR_MANAGER_H_

#include "htp.h"
#include "network_inspector/network_inspector.h"
#include "socket.h"

#include <Windows.h>
#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <set>

namespace NetworkInspector
{
    enum class Lock
    {
        SocketLock,
        RecvLock,
        RecvFromLock,
        AcceptLock,
        LOCK_NB_ELEM
    };

    enum class Context
    {
        SocketContext,
        RecvContext,
        RecvFromContext,
        AcceptContext,
        CONTEXT_NB_ELEM
    };

    struct SocketContext
    {
        int af;
        int type;
        unsigned long ip_addr;
        unsigned short port;
    };

    struct AcceptContext
    {
        SOCKET listening_socket;
        sockaddr* addr;
        int* addrlen;
        sockaddr temp_addr;
        int temp_addrlen;
    };

    struct RecvContext
    {
        SOCKET s;
        const char* buf;
        int len;
        int flags;
        std::vector<uintptr_t> backtrace;
    };

    struct RecvFromContext
    {
        SOCKET s;
        const char* buf;
        int len;
        int flags;
        const sockaddr* from;
        int* fromlen;
        sockaddr temp_addr;
        int temp_addrlen;
        std::vector<uintptr_t> backtrace;
    };

    class NetworkInspectorManager
    {
    public:
        NetworkInspectorManager();
        ~NetworkInspectorManager();

        void Init();
        void Init(HTPHandle* htp);
        void Cleanup();
        void Start();
        void Stop();

        void AddIPAddressExclusion(const char* ip_addr);
        void RemoveIPAddressExclusion(const char* ip_addr);

        void AddPortExclusion(unsigned short port);
        void RemovePortExclusion(unsigned short port);

        void RegisterCallback(std::function<void(NetworkContext&)> cb);

        bool IsIpAddressExcluded(unsigned long ip_addr);
        bool IsPortExcluded(unsigned short port);
        bool IsSocketTracked(const Socket& s);
        bool IsSocketAddressTracked(const sockaddr_in* addr);
        bool IsSocketAddressTracked(unsigned long ip_addr, unsigned short port);

        void CallCallback(NetworkContext& ctx);

        void AcquireLock(Lock type);
        void ReleaseLock(Lock type);
        void SetContext(Context type, std::shared_ptr<void> ctx);
        std::shared_ptr<void> GetContext(Context type);

        uintptr_t GetExportedFunctionAddress(const char* module, const char* proc);

        void AddSocket(uintptr_t socket_id, std::shared_ptr<Socket> socket);
        void RemoveSocket(uintptr_t socket_id);

        std::shared_ptr<Socket> GetSocket(uintptr_t socket_id);
        std::vector<uintptr_t> GetBacktrace(uintptr_t rip, uintptr_t rsp);

        static NetworkInspectorManager& GetInstance()
        {
            static NetworkInspectorManager instance;
            return instance;
        }

    private:
        void AddHooks();
        void RemoveHooks();

        bool m_is_started;
        bool m_is_htp_owner;
        std::shared_ptr<void> m_contexts[static_cast<int>(Context::CONTEXT_NB_ELEM)];
        std::mutex m_locks[static_cast<int>(Lock::LOCK_NB_ELEM)];
        std::set<unsigned long> m_excluded_ips;
        std::set<unsigned short> m_excluded_ports;
        std::map<uintptr_t, std::shared_ptr<Socket>> m_sockets;
        HTPHandle* m_htp;
        std::function<void(NetworkContext&)> m_cb;
    };

} // namespace NetworkInspector

#endif // NETWORK_INSPECTOR_MANAGER_H_
