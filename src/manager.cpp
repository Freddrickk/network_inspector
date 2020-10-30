#include "manager.h"

#include "hooks.h"
#include "htp.h"
#include "utils.h"

#include <vector>

namespace NetworkInspector
{
    NetworkInspectorManager::NetworkInspectorManager()
        : m_is_started(false), m_is_htp_owner(false), m_locks(), m_callback_mutex(), m_contexts(), m_htp()
    {
    }

    NetworkInspectorManager::~NetworkInspectorManager()
    {
    }

    void NetworkInspectorManager::Init(HTPHandle* htp)
    {
        m_htp          = htp;
        m_is_htp_owner = false;
    }

    void NetworkInspectorManager::Init()
    {
        m_htp          = HTPInit();
        m_is_htp_owner = true;
    }

    void NetworkInspectorManager::Cleanup()
    {
        if (m_is_htp_owner && m_htp != nullptr)
            HTPClose(m_htp);
    }

    void NetworkInspectorManager::AddIPAddressExclusion(const char* ip_addr)
    {
        m_excluded_ips.insert(inet_addr(ip_addr));
    }

    void NetworkInspectorManager::RemoveIPAddressExclusion(const char* ip_addr)
    {
        m_excluded_ips.erase(inet_addr(ip_addr));
    }

    void NetworkInspectorManager::AddPortExclusion(unsigned short port)
    {
        m_excluded_ports.insert(port);
    }

    void NetworkInspectorManager::RemovePortExclusion(unsigned short port)
    {
        m_excluded_ports.erase(port);
    }

    bool NetworkInspectorManager::IsIpAddressExcluded(unsigned long ip_addr)
    {
        return m_excluded_ips.find(ip_addr) != m_excluded_ips.end();
    }

    bool NetworkInspectorManager::IsPortExcluded(unsigned short port)
    {
        return m_excluded_ports.find(port) != m_excluded_ports.end();
    }

    bool NetworkInspectorManager::IsSocketTracked(const Socket& s)
    {
        return !IsIpAddressExcluded(s.GetIPPacked()) && !IsPortExcluded(s.GetPort());
    }

    bool NetworkInspectorManager::IsSocketAddressTracked(const sockaddr_in* addr)
    {
        return !IsIpAddressExcluded(addr->sin_addr.s_addr) && !IsPortExcluded(ntohs(addr->sin_port));
    }

    bool NetworkInspectorManager::IsSocketAddressTracked(unsigned long ip_addr, unsigned short port)
    {
        return !IsIpAddressExcluded(ip_addr) && !IsPortExcluded(port);
    }

    void NetworkInspectorManager::RegisterCallback(std::function<void(NetworkContext&)> cb)
    {
        m_cb = cb;
    }

    void NetworkInspectorManager::CallCallback(NetworkContext& ctx)
    {
        std::lock_guard<std::mutex> lock(m_callback_mutex);
        m_cb(ctx);
    }

    void NetworkInspectorManager::Start()
    {
        if (m_is_started)
            return;

        AddHooks();
        m_is_started = true;
    }

    void NetworkInspectorManager::Stop()
    {
        if (!m_is_started)
            return;

        RemoveHooks();
        m_is_started = false;
    }

    void NetworkInspectorManager::AcquireLock(Lock type)
    {
        int type_idx = static_cast<int>(type);
        if (type_idx < static_cast<int>(Lock::LOCK_NB_ELEM))
            m_locks[type_idx].lock();
    }

    void NetworkInspectorManager::ReleaseLock(Lock type)
    {
        int type_idx = static_cast<int>(type);
        if (type_idx < static_cast<int>(Lock::LOCK_NB_ELEM))
            m_locks[type_idx].unlock();
    }

    void NetworkInspectorManager::SetContext(Context type, std::shared_ptr<void> ctx)
    {
        int type_idx = static_cast<int>(type);
        if (type_idx < static_cast<int>(Context::CONTEXT_NB_ELEM))
            m_contexts[type_idx] = ctx;
    }

    std::shared_ptr<void> NetworkInspectorManager::GetContext(Context type)
    {
        int type_idx = static_cast<int>(type);
        return type_idx < static_cast<int>(Context::CONTEXT_NB_ELEM) ? m_contexts[type_idx] : nullptr;
    }

    uintptr_t NetworkInspectorManager::GetExportedFunctionAddress(const char* module_name, const char* proc_name)
    {
        if (module_name == nullptr || proc_name == nullptr)
            return 0;

        HMODULE module = GetModuleHandleA(module_name);
        if (!module)
            return 0;

        return (uintptr_t)GetProcAddress(module, proc_name);
    }

    void NetworkInspectorManager::AddHooks()
    {
        if (m_is_started)
            return;

        HMODULE module = GetModuleHandleA("ws2_32.dll");
        if (!module)
            return;

        void* fn_socket      = (void*)GetProcAddress(module, "socket");
        void* fn_connect     = (void*)GetProcAddress(module, "connect");
        void* fn_closesocket = (void*)GetProcAddress(module, "closesocket");
        void* fn_recv        = (void*)GetProcAddress(module, "recv");
        void* fn_recvfrom    = (void*)GetProcAddress(module, "recvfrom");
        void* fn_send        = (void*)GetProcAddress(module, "send");
        void* fn_sendto      = (void*)GetProcAddress(module, "sendto");
        void* fn_bind        = (void*)GetProcAddress(module, "bind");
        void* fn_accept      = (void*)GetProcAddress(module, "accept");

        if (fn_socket != nullptr)
            SetupInlineHook(m_htp, (uintptr_t)fn_socket, Hooks::HookSocket, Hooks::HookSocketPost);

        if (fn_connect != nullptr)
            SetupInlineHook(m_htp, (uintptr_t)fn_connect, Hooks::HookConnect);

        if (fn_closesocket != nullptr)
            SetupInlineHook(m_htp, (uintptr_t)fn_closesocket, Hooks::HookCloseSocket);

        if (fn_recv != nullptr)
            SetupInlineHook(m_htp, (uintptr_t)fn_recv, Hooks::HookRecv, Hooks::HookRecvPost);

        if (fn_recvfrom != nullptr)
            SetupInlineHook(m_htp, (uintptr_t)fn_recvfrom, Hooks::HookRecvFrom, Hooks::HookRecvFromPost);

        if (fn_send != nullptr)
            SetupInlineHook(m_htp, (uintptr_t)fn_send, Hooks::HookSend);

        if (fn_sendto != nullptr)
            SetupInlineHook(m_htp, (uintptr_t)fn_sendto, Hooks::HookSendTo);

        if (fn_bind != nullptr)
            SetupInlineHook(m_htp, (uintptr_t)fn_bind, Hooks::HookBind);

        if (fn_accept != nullptr)
            SetupInlineHook(m_htp, (uintptr_t)fn_accept, Hooks::HookAccept, Hooks::HookAcceptPost);
    }

    void NetworkInspectorManager::RemoveHooks()
    {
        if (!m_is_started)
            return;

        HMODULE module = GetModuleHandleA("ws2_32.dll");
        if (!module)
            return;

        void* fn_socket      = (void*)GetProcAddress(module, "socket");
        void* fn_connect     = (void*)GetProcAddress(module, "connect");
        void* fn_closesocket = (void*)GetProcAddress(module, "closesocket");
        void* fn_recv        = (void*)GetProcAddress(module, "recv");
        void* fn_recvfrom    = (void*)GetProcAddress(module, "recvfrom");
        void* fn_send        = (void*)GetProcAddress(module, "send");
        void* fn_sendto      = (void*)GetProcAddress(module, "sendto");
        void* fn_bind        = (void*)GetProcAddress(module, "bind");
        void* fn_accept      = (void*)GetProcAddress(module, "accept");

        if (fn_socket != nullptr)
            RemoveInlineHook(m_htp, (uintptr_t)fn_socket);

        if (fn_connect != nullptr)
            RemoveInlineHook(m_htp, (uintptr_t)fn_connect);

        if (fn_closesocket != nullptr)
            RemoveInlineHook(m_htp, (uintptr_t)fn_closesocket);

        if (fn_recv != nullptr)
            RemoveInlineHook(m_htp, (uintptr_t)fn_recv);

        if (fn_recvfrom != nullptr)
            RemoveInlineHook(m_htp, (uintptr_t)fn_recvfrom);

        if (fn_send != nullptr)
            RemoveInlineHook(m_htp, (uintptr_t)fn_send);

        if (fn_sendto != nullptr)
            RemoveInlineHook(m_htp, (uintptr_t)fn_sendto);

        if (fn_bind != nullptr)
            RemoveInlineHook(m_htp, (uintptr_t)fn_bind);

        if (fn_accept != nullptr)
            RemoveInlineHook(m_htp, (uintptr_t)fn_bind);
    }

    void NetworkInspectorManager::AddSocket(uintptr_t socket_id, std::shared_ptr<Socket> socket)
    {
        if (socket)
            m_sockets[socket_id] = socket;
    }

    void NetworkInspectorManager::RemoveSocket(uintptr_t socket_id)
    {
        auto it = m_sockets.find(socket_id);
        if (it != m_sockets.end())
            m_sockets.erase(it);
    }

    std::shared_ptr<Socket> NetworkInspectorManager::GetSocket(uintptr_t socket_id)
    {
        auto it = m_sockets.find(socket_id);
        if (it != m_sockets.end())
            return it->second;
        return {};
    }

    std::vector<uintptr_t> NetworkInspectorManager::GetBacktrace(uintptr_t rip, uintptr_t rsp)
    {
        uintptr_t retaddrs[0x20] = {};
        int size                 = ProcessGetBacktrace(rip, rsp, retaddrs, 0x20);
        auto result              = std::vector<uintptr_t>();
        for (int i = 0; i < size; i++)
            result.push_back(retaddrs[i]);
        return result;
    }

} // namespace NetworkInspector
