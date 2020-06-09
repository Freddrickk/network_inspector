#include "network_inspector/network_inspector.h"

#include "manager.h"

namespace NetworkInspector
{
    static bool is_initialized = false;

    void Init()
    {
        if (!is_initialized)
        {
            NetworkInspectorManager::GetInstance().Init();
            is_initialized = true;
        }
    }

    void Init(HTPHandle* htp)
    {
        if (!is_initialized)
        {
            NetworkInspectorManager::GetInstance().Init(htp);
            is_initialized = true;
        }
    }

    void Cleanup()
    {
        if (is_initialized)
            NetworkInspectorManager::GetInstance().Cleanup();
    }

    void Start()
    {
        if (is_initialized)
            NetworkInspectorManager::GetInstance().Start();
    }

    void Stop()
    {
        if (is_initialized)
            NetworkInspectorManager::GetInstance().Stop();
    }

    void AddIpAddressExclusion(const char* ip)
    {
        if (is_initialized)
            NetworkInspectorManager::GetInstance().AddIPAddressExclusion(ip);
    }

    void RemoveIpAddressExclusion(const char* ip)
    {
        if (is_initialized)
            NetworkInspectorManager::GetInstance().RemoveIPAddressExclusion(ip);
    }

    void AddPortExclusion(unsigned short port)
    {
        if (is_initialized)
            NetworkInspectorManager::GetInstance().AddPortExclusion(port);
    }

    void RemovePortExclusion(unsigned short port)
    {
        if (is_initialized)
            NetworkInspectorManager::GetInstance().RemovePortExclusion(port);
    }

    void RegisterCallback(std::function<void(NetworkContext&)> callback)
    {
        if (is_initialized)
            NetworkInspectorManager::GetInstance().RegisterCallback(callback);
    }

} // namespace NetworkInspector
