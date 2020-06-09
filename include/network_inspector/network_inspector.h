#ifndef NETWORK_INSPECTOR_H_
#define NETWORK_INSPECTOR_H_

#include "htp.h"

#include <functional>
#include <string>
#include <vector>

namespace NetworkInspector
{
    enum class ContextType
    {
        RecvCallback,
        SendCallback,
        RecvFromCallback,
        SendToCallback
    };

    enum class SocketType
    {
        SocketUDP,
        SocketTCP,
        SocketUnknown
    };

    struct NetworkContext
    {
        /* Describes the type of API that was called */
        ContextType type;

        /* Describes the type of socket used */
        SocketType socket_type;

        /* Contains the IP address of the remote endpoint */
        std::string ip;

        /* Contains the port used for the network communication */
        unsigned short port;

        /* Contains the buffer sent/received. */
        /* This data can be modified at will although changing this pointer will only work when sending data */
        unsigned char* buffer;

        /* Contains the size of the buffer */
        /* This value can be modified and must match the size of the data in the buffer. */
        int size;

        /* Contains the backtrace when API was called (send, recv, sendto, recvfrom) */
        std::vector<uintptr_t> backtrace;

        /* Contains the size of the buffer on receive operations (RecvCallback and RecvFromCallback) */
        int recv_buffer_size;
    };

    /* Initialize Network Inspector */
    void Init();

    /* Initialize Network Inspector with a given HTP handle */
    /* This is useful if you're already using HTP in your project */
    void Init(HTPHandle* htp);

    /* Start Network Inspector and setup hooks */
    void Start();

    /* Stop Network Inspector and remove hooks */
    void Stop();

    /* Add an IP address to be excluded meaning the callback won't be called for this endpoint */
    void AddIpAddressExclusion(const char* ip);

    /* Remove an excluded IP address */
    void RemoveIpAddressExclusion(const char* ip);

    /* Add a port to be excluded meaning the callback won't be called for this port */
    /* i.e. Ignore TLS connections by excluding port 443 */
    void AddPortExclusion(unsigned short port);

    /* Remove an excluded port */
    void RemovePortExclusion(unsigned short port);

    /* Register the callback that will be called when network communication occurs */
    void RegisterCallback(std::function<void(NetworkContext&)> callback);

    /* Release resources used by Network Inspector */
    void Cleanup();

} // namespace NetworkInspector

#endif // NETWORK_INSPECTOR_H_
