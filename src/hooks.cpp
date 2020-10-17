#include "hooks.h"

#include "manager.h"

#include <Windows.h>

#ifdef _M_X64
    #define GET_RETVAL(ctx)     (ctx->rax)
    #define GET_RETVAL_REF(ctx) (&ctx->rax)
    #define GET_ARG_1(ctx)      (ctx->rcx)
    #define GET_ARG_1_REF(ctx)  (&ctx->rcx)
    #define GET_ARG_2(ctx)      (ctx->rdx)
    #define GET_ARG_2_REF(ctx)  (&ctx->rdx)
    #define GET_ARG_3(ctx)      (ctx->r8)
    #define GET_ARG_3_REF(ctx)  (&ctx->r8)
    #define GET_ARG_4(ctx)      (ctx->r9)
    #define GET_ARG_4_REF(ctx)  (&ctx->r9)
    #define GET_ARG_5(ctx)      (*(uint64_t*)(ctx->rsp + 0x28 + 0x00))
    #define GET_ARG_5_REF(ctx)  ((uint64_t*)(ctx->rsp + 0x28 + 0x00))
    #define GET_ARG_6(ctx)      (*(uint64_t*)(ctx->rsp + 0x28 + 0x08))
    #define GET_ARG_6_REF(ctx)  ((uint64_t*)(ctx->rsp + 0x28 + 0x08))
    #define GET_SP(ctx)         (ctx->rsp)
#else
    #define GET_RETVAL(ctx)     (ctx->eax)
    #define GET_RETVAL_REF(ctx) (&ctx->eax)
    #define GET_ARG_1(ctx)      (*(uint32_t*)(ctx->esp + 0x04 + 0x00))
    #define GET_ARG_1_REF(ctx)  ((uint32_t*)(ctx->esp + 0x04 + 0x00))
    #define GET_ARG_2(ctx)      (*(uint32_t*)(ctx->esp + 0x04 + 0x04))
    #define GET_ARG_2_REF(ctx)  ((uint32_t*)(ctx->esp + 0x04 + 0x04))
    #define GET_ARG_3(ctx)      (*(uint32_t*)(ctx->esp + 0x04 + 0x08))
    #define GET_ARG_3_REF(ctx)  ((uint32_t*)(ctx->esp + 0x04 + 0x08))
    #define GET_ARG_4(ctx)      (*(uint32_t*)(ctx->esp + 0x04 + 0x0c))
    #define GET_ARG_4_REF(ctx)  ((uint32_t*)(ctx->esp + 0x04 + 0x0c))
    #define GET_ARG_5(ctx)      (*(uint32_t*)(ctx->esp + 0x04 + 0x10))
    #define GET_ARG_5_REF(ctx)  ((uint32_t*)(ctx->esp + 0x04 + 0x10))
    #define GET_ARG_6(ctx)      (*(uint32_t*)(ctx->esp + 0x04 + 0x14))
    #define GET_ARG_6_REF(ctx)  ((uint32_t*)(ctx->esp + 0x04 + 0x14))
    #define GET_SP(ctx)         (ctx->esp)
#endif

namespace NetworkInspector
{
    namespace Hooks
    {
        void HookSocket(HTPContext* ctx)
        {
            int af       = (int)GET_ARG_1(ctx);
            int type     = (int)GET_ARG_2(ctx);
            int protocol = (int)GET_ARG_3(ctx);
            auto& ni     = NetworkInspectorManager::GetInstance();

            // Make sure only one thread at a time can enter socket
            ni.AcquireLock(Lock::SocketLock);

            if (af == AF_INET)
            {
                if (type == SOCK_STREAM || type == SOCK_DGRAM)
                {
                    auto sd     = std::make_shared<SocketContext>();
                    sd->af      = af;
                    sd->type    = type;
                    sd->ip_addr = 0;
                    sd->port    = 0;
                    // Add the socket info in the socket context and retrieve it in the post hook
                    // where the socket id will be available
                    ni.SetContext(Context::SocketContext, sd);
                }
            }
        }

        void HookSocketPost(HTPContext* ctx)
        {
            auto& ni = NetworkInspectorManager::GetInstance();
            auto sd  = std::static_pointer_cast<SocketContext>(ni.GetContext(Context::SocketContext));

            // if the socket is IPv4 and UDP or TCP, retrieve the socket information and add it to the sockets map
            if (sd)
            {
                uintptr_t socket_id = (uintptr_t)GET_RETVAL(ctx);
                auto socket         = std::make_shared<Socket>(socket_id, sd->af, sd->type);
                NetworkInspectorManager::GetInstance().AddSocket(socket_id, socket);
            }

            ni.SetContext(Context::SocketContext, nullptr);
            ni.ReleaseLock(Lock::SocketLock);
        }

        void HookConnect(HTPContext* ctx)
        {
            SOCKET s                = (SOCKET)GET_ARG_1(ctx);
            const sockaddr_in* name = (const sockaddr_in*)GET_ARG_2(ctx);
            int namelen             = (int)GET_ARG_3(ctx);

            auto& ni    = NetworkInspectorManager::GetInstance();
            auto socket = ni.GetSocket(s);
            if (socket)
            {
                socket->SetIPPacked(name->sin_addr.s_addr);
                socket->SetPortPacked(name->sin_port);
            }
        }

        void HookBind(HTPContext* ctx)
        {
            SOCKET s             = (SOCKET)GET_ARG_1(ctx);
            const sockaddr* addr = (const sockaddr*)GET_ARG_2(ctx);
            int namelen          = (int)GET_ARG_3(ctx);

            auto& ni    = NetworkInspectorManager::GetInstance();
            auto socket = ni.GetSocket(s);

            if (socket && socket->GetType() == SocketType::SocketUDP)
            {
                const sockaddr_in* addr_in = (sockaddr_in*)addr;
                socket->SetPortPacked(addr_in->sin_port);
                socket->SetIPPacked(addr_in->sin_addr.s_addr);
            }
        }

        void HookAccept(HTPContext* ctx)
        {
            SOCKET s       = (SOCKET)GET_ARG_1(ctx);
            sockaddr* addr = (sockaddr*)GET_ARG_2(ctx);
            int* addrlen   = (int*)GET_ARG_3(ctx);

            auto& ni = NetworkInspectorManager::GetInstance();

            // Make sure only one thread at a time can enter socket
            ni.AcquireLock(Lock::AcceptLock);

            auto ad = std::make_shared<AcceptContext>();

            // We want to retrieve the socket information even if the caller doesn't want it
            addr             = addr != nullptr ? addr : &ad->temp_addr;
            ad->temp_addrlen = sizeof(ad->temp_addr);
            addrlen          = addrlen != nullptr ? addrlen : &ad->temp_addrlen;

            *GET_ARG_2_REF(ctx) = (uintptr_t)addr;
            *GET_ARG_3_REF(ctx) = (uintptr_t)addrlen;

            ad->listening_socket = s;
            ad->addr             = addr;
            ad->addrlen          = addrlen;
            ni.SetContext(Context::AcceptContext, ad);
        }

        void HookAcceptPost(HTPContext* ctx)
        {
            SOCKET conn_socket = (SOCKET)GET_RETVAL(ctx);

            auto& ni = NetworkInspectorManager::GetInstance();

            if (conn_socket != INVALID_SOCKET)
            {
                auto ad = std::static_pointer_cast<AcceptContext>(ni.GetContext(Context::AcceptContext));
                if (ad != nullptr && ad->addr->sa_family == AF_INET)
                {
                    uintptr_t socket_id = (uintptr_t)conn_socket;
                    sockaddr_in* addr   = (sockaddr_in*)ad->addr;

                    auto lsocket           = ni.GetSocket(ad->listening_socket);
                    SocketType socket_type = SocketType::SocketUnknown;
                    unsigned short port    = 0;
                    if (lsocket != nullptr)
                    {
                        socket_type = lsocket->GetType();
                        if (lsocket->GetPort() == 0)
                            lsocket->SetPortNumber(Socket::GetPortFromNativeSocket(ad->listening_socket));
                        port = lsocket->GetPort();
                    }
                    else
                    {
                        socket_type = Socket::GetTypeFromNativeSocket(conn_socket);
                    }

                    auto socket = std::make_shared<Socket>(socket_id);
                    socket->SetAddressFamily(addr->sin_family);
                    socket->SetType(socket_type);
                    socket->SetIPPacked(addr->sin_addr.s_addr);
                    socket->SetPortNumber(port);
                    NetworkInspectorManager::GetInstance().AddSocket(socket_id, socket);
                }
            }

            ni.SetContext(Context::AcceptContext, nullptr);
            ni.ReleaseLock(Lock::AcceptLock);
        }

        void HookCloseSocket(HTPContext* ctx)
        {
            SOCKET s = (SOCKET)GET_ARG_1(ctx);

            auto& ni = NetworkInspectorManager::GetInstance();
            ni.RemoveSocket(s);
        }

        void HookSend(HTPContext* ctx)
        {
            SOCKET s           = (SOCKET)GET_ARG_1(ctx);
            unsigned char* buf = (unsigned char*)GET_ARG_2(ctx);
            int len            = (int)GET_ARG_3(ctx);
            int flags          = (int)GET_ARG_4(ctx);

            auto& ni    = NetworkInspectorManager::GetInstance();
            auto socket = ni.GetSocket(s);
            if (socket && ni.IsSocketTracked(*socket) && len >= 0)
            {
                NetworkContext context   = {};
                context.type             = ContextType::SendCallback;
                context.socket_type      = socket->GetType();
                context.ip               = socket->GetIPString();
                context.port             = socket->GetPort();
                context.buffer           = buf;
                context.size             = (size_t)len;
                context.recv_buffer_size = 0;
                context.backtrace        = ni.GetBacktrace(ni.GetExportedFunctionAddress("ws2_32.dll", "send"), GET_SP(ctx));

                ni.CallCallback(context);

                // Allows the callback to modify the buffer ptr
                *GET_ARG_2_REF(ctx) = (uintptr_t)context.buffer;
                // Allows the callback to modify the buffer size
                *GET_ARG_3_REF(ctx) = (int)context.size;
            }
        }

        void HookSendTo(HTPContext* ctx)
        {
            SOCKET s              = (SOCKET)GET_ARG_1(ctx);
            unsigned char* buf    = (unsigned char*)GET_ARG_2(ctx);
            int len               = (int)GET_ARG_3(ctx);
            int flags             = (int)GET_ARG_4(ctx);
            const sockaddr_in* to = (const sockaddr_in*)GET_ARG_5(ctx);
            int tolen             = (int)GET_ARG_6(ctx);

            auto& ni    = NetworkInspectorManager::GetInstance();
            auto socket = ni.GetSocket(s);
            if (socket && len >= 0)
            {
                const unsigned short udp_port = socket->GetPort() == 0 && to != nullptr ? ntohs(to->sin_port) : socket->GetPort();
                if ((socket->GetType() == SocketType::SocketTCP && ni.IsSocketTracked(*socket)) ||
                    (socket->GetType() == SocketType::SocketUDP && to != nullptr && ni.IsSocketAddressTracked(to->sin_addr.s_addr, udp_port)))
                {
                    NetworkContext context = {};
                    context.type           = ContextType::SendToCallback;
                    context.socket_type    = socket->GetType();
                    if (context.socket_type == SocketType::SocketUDP && to != nullptr)
                    {
                        context.ip   = Socket::GetIPAddressStringFromPacked(to->sin_addr.s_addr);
                        context.port = udp_port;
                    }
                    else
                    {
                        context.ip   = socket->GetIPString();
                        context.port = socket->GetPort();
                    }
                    context.buffer           = buf;
                    context.size             = (size_t)len;
                    context.recv_buffer_size = 0;
                    context.backtrace        = ni.GetBacktrace(ni.GetExportedFunctionAddress("ws2_32.dll", "sendto"), GET_SP(ctx));

                    ni.CallCallback(context);

                    // Allows the callback to modify the buffer ptr
                    *GET_ARG_2_REF(ctx) = (uintptr_t)context.buffer;
                    // Allows the callback to modify the buffer size
                    *GET_ARG_3_REF(ctx) = (int)context.size;
                }
            }
        }

        void HookRecv(HTPContext* ctx)
        {
            SOCKET s        = (SOCKET)GET_ARG_1(ctx);
            const char* buf = (const char*)GET_ARG_2(ctx);
            int len         = (int)GET_ARG_3(ctx);
            int flags       = (int)GET_ARG_4(ctx);

            auto& ni = NetworkInspectorManager::GetInstance();

            // Make sure only one thread at a time can enter recv
            ni.AcquireLock(Lock::RecvLock);


            auto socket = ni.GetSocket(s);
            if (socket && ni.IsSocketTracked(*socket))
            {
                auto rd       = std::make_shared<RecvContext>();
                rd->s         = s;
                rd->buf       = buf;
                rd->len       = len;
                rd->flags     = flags;
                rd->backtrace = ni.GetBacktrace(ni.GetExportedFunctionAddress("ws2_32.dll", "recv"), GET_SP(ctx));
                ni.SetContext(Context::RecvContext, rd);
            }
        }

        void HookRecvPost(HTPContext* ctx)
        {
            auto& ni = NetworkInspectorManager::GetInstance();

            auto rd               = std::static_pointer_cast<RecvContext>(ni.GetContext(Context::RecvContext));
            int nb_bytes_received = (int)GET_RETVAL(ctx);
            if (nb_bytes_received >= 0 && rd)
            {
                auto socket = ni.GetSocket(rd->s);
                if (socket && rd->len >= 0)
                {
                    NetworkContext context   = {};
                    context.type             = ContextType::RecvCallback;
                    context.socket_type      = socket->GetType();
                    context.ip               = socket->GetIPString();
                    context.port             = socket->GetPort();
                    context.buffer           = (unsigned char*)rd->buf;
                    context.size             = (size_t)nb_bytes_received;
                    context.recv_buffer_size = (size_t)rd->len;
                    context.backtrace        = rd->backtrace;

                    ni.CallCallback(context);

                    // Allows to modify the number of bytes received in the callback
                    *GET_RETVAL_REF(ctx) = context.size;
                }
            }

            ni.SetContext(Context::RecvContext, nullptr);
            ni.ReleaseLock(Lock::RecvLock);
        }

        void HookRecvFrom(HTPContext* ctx)
        {
            SOCKET s             = (SOCKET)GET_ARG_1(ctx);
            const char* buf      = (const char*)GET_ARG_2(ctx);
            int len              = (int)GET_ARG_3(ctx);
            int flags            = (int)GET_ARG_4(ctx);
            const sockaddr* from = (const sockaddr*)GET_ARG_5(ctx);
            int* fromlen         = (int*)GET_ARG_6(ctx);

            auto& ni = NetworkInspectorManager::GetInstance();

            // Make sure only one thread at a time can enter recvfrom
            ni.AcquireLock(Lock::RecvFromLock);

            auto rd = std::make_shared<RecvFromContext>();

            auto socket = ni.GetSocket(s);
            if (socket && socket->GetType() == SocketType::SocketUDP)
            {
                // We want to make to get the socket info if the ignores it (from == NULL)
                from             = from != nullptr ? from : &rd->temp_addr;
                rd->temp_addrlen = sizeof(rd->temp_addr);
                fromlen          = fromlen != nullptr ? fromlen : &rd->temp_addrlen;

                *GET_ARG_5_REF(ctx) = (uintptr_t)from;
                *GET_ARG_6_REF(ctx) = (uintptr_t)fromlen;
            }

            rd->s         = s;
            rd->buf       = buf;
            rd->len       = len;
            rd->flags     = flags;
            rd->from      = from;
            rd->fromlen   = fromlen;
            rd->backtrace = ni.GetBacktrace(ni.GetExportedFunctionAddress("ws2_32.dll", "recvfrom"), GET_SP(ctx));

            ni.SetContext(Context::RecvFromContext, rd);
        }

        void HookRecvFromPost(HTPContext* ctx)
        {
            auto& ni = NetworkInspectorManager::GetInstance();
            auto rd  = std::static_pointer_cast<RecvFromContext>(ni.GetContext(Context::RecvFromContext));
            // Return value of recvfrom
            int nb_bytes_received = (int)GET_RETVAL(ctx);
            if (nb_bytes_received >= 0 && rd)
            {
                auto socket = ni.GetSocket(rd->s);
                if (socket && rd->len >= 0)
                {
                    const sockaddr_in* addr_from = (sockaddr_in*)rd->from;
                    const unsigned short udp_port =
                        socket->GetPort() == 0 && addr_from != nullptr ? ntohs(addr_from->sin_port) : socket->GetPort();
                    if ((socket->GetType() == SocketType::SocketTCP && ni.IsSocketTracked(*socket)) ||
                        (socket->GetType() == SocketType::SocketUDP && addr_from != nullptr &&
                         ni.IsSocketAddressTracked(addr_from->sin_addr.s_addr, udp_port)))
                    {
                        NetworkContext context = {};
                        context.type           = ContextType::RecvFromCallback;
                        context.socket_type    = socket->GetType();
                        if (context.socket_type == SocketType::SocketUDP && addr_from != nullptr)
                        {
                            context.ip   = Socket::GetIPAddressStringFromPacked(addr_from->sin_addr.s_addr);
                            context.port = udp_port;
                        }
                        else
                        {
                            context.ip   = socket->GetIPString();
                            context.port = socket->GetPort();
                        }

                        context.buffer           = (unsigned char*)rd->buf;
                        context.size             = (size_t)nb_bytes_received;
                        context.recv_buffer_size = (size_t)rd->len;
                        context.backtrace        = rd->backtrace;

                        ni.CallCallback(context);

                        // Allows to modify the number of bytes received in the callback
                        *GET_RETVAL_REF(ctx) = context.size;
                    }
                }
            }

            ni.SetContext(Context::RecvFromContext, nullptr);
            ni.ReleaseLock(Lock::RecvFromLock);
        }
    } // namespace Hooks
} // namespace NetworkInspector
