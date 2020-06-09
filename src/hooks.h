#ifndef NETWORK_INSPECTOR_HOOKS_H_
#define NETWORK_INSPECTOR_HOOKS_H_

#include "htp.h"

namespace NetworkInspector
{
    namespace Hooks
    {
        void HookSocket(HTPContext* ctx);
        void HookSocketPost(HTPContext* ctx);
        void HookConnect(HTPContext* ctx);
        void HookBind(HTPContext* ctx);
        void HookAccept(HTPContext* ctx);
        void HookAcceptPost(HTPContext* ctx);
        void HookCloseSocket(HTPContext* ctx);
        void HookSend(HTPContext* ctx);
        void HookSendTo(HTPContext* ctx);
        void HookRecv(HTPContext* ctx);
        void HookRecvPost(HTPContext* ctx);
        void HookRecvFrom(HTPContext* ctx);
        void HookRecvFromPost(HTPContext* ctx);
    } // namespace Hooks
} // namespace NetworkInspector

#endif // NETWORK_INSPECTOR_HOOKS_H_
