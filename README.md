# Network Inspector

## Description

A small SDK to manipulate TCP/UDP network packets in-process.

## Build

```
git clone git@github.com:Freddrickk/network_inspector.git
cd network_inspector

git submodule update --init --recursive
mkdir cmake64
cd cmake64

cmake build -A x64 ..
cmake --build .
```

Note: Use `cmake build -A Win32 ..` to build the 32-bit version

## Usage

This is an example of a callback:

```cpp
#include <cstdio>

#include "network_inspector/network_inspector.h"

void NetworkInspectorCallback(NetworkInspector::NetworkContext& nc)
{
    if (nc.type == NetworkInspector::ContextType::RecvCallback)
    {
        printf("recv => IP: %s Port: %hu Buffer: %p Size=%#llx RecvBufferSize=%#llx\n",
               nc.ip.c_str(),
               nc.port,
               nc.buffer,
               nc.size,
               nc.recv_buffer_size);
    }
    else if (nc.type == NetworkInspector::ContextType::RecvFromCallback)
    {
        printf("recvfrom => IP: %s Port: %hu Buffer: %p Size=%#llx RecvBufferSize=%#llx\n",
               nc.ip.c_str(),
               nc.port,
               nc.buffer,
               nc.size,
               nc.recv_buffer_size);
    }
    else if (nc.type == NetworkInspector::ContextType::SendCallback)
    {
        printf("send => IP: %s Port: %hu Buffer: %p Size=%#llx\n",
               nc.ip.c_str(),
               nc.port,
               nc.buffer,
               nc.size);
    }
    else if (nc.type == NetworkInspector::ContextType::SendToCallback)
    {
        printf("sendto => IP: %s Port: %hu Buffer: %p Size=%#llx\n",
               nc.ip.c_str(),
               nc.port,
               nc.buffer,
               nc.size);
    }

    // Access the backtrace of the network API call
    for (auto& addr : nc.backtrace)
        printf("%p\n", (void*)addr);
}

void StartNetworkInspector()
{
    NetworkInspector::Init();
    // You can exclude port numbers from calling the callback
    NetworkInspector::AddPortExclusion(443);
    // You can exclude IP addresses from calling the callback
    NetworkInspector::AddIpAddressExclusion("127.0.0.1");
    NetworkInspector::RegisterCallback(NetworkInspectorCallback);
    NetworkInspector::Start();
};

void StopNetworkInspector()
{
    NetworkInspector::Stop();
    NetworkInspector::Cleanup();
}
```

## Limitations

- Only supports network communication based on Linux-style socket usage (socket/send/recv/accept/listen/bind/connect)
- Windows only