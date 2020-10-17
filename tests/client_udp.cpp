#include "network_inspector/network_inspector.h"
#include "test_utils.h"

#include <Windows.h>
#include <cassert>
#include <cstdio>

constexpr char* ip_addr       = "127.0.0.1";
constexpr unsigned short port = 31337;

size_t test_number = 0;

void NetworkInspectorCallback(NetworkInspector::NetworkContext& nc)
{
    switch (test_number)
    {
        case 0:
        {
            TEST_EQ(nc.socket_type, NetworkInspector::SocketType::SocketUDP, "Socket should be of SocketUDP type");
            TEST_EQ(nc.type, NetworkInspector::ContextType::SendToCallback, "First packet sent should be of type ContextType::SendToCallback");
            TEST_EQ(nc.port, port, "The port number should match");
            TEST_STR(nc.ip.c_str(), ip_addr, "Ip address should match");
            TEST_MEM(nc.buffer, nc.size, udp_client_packet_1, sizeof(udp_client_packet_1), "First packet buffer should match");
            break;
        }
        case 1:
        {
            TEST_EQ(nc.socket_type, NetworkInspector::SocketType::SocketUDP, "Socket should be of SocketUDP type");
            TEST_EQ(nc.type,
                    NetworkInspector::ContextType::RecvFromCallback,
                    "First packet received should be of type ContextType::RecvFromCallback");
            TEST_EQ(nc.port, port, "The port number should match");
            TEST_STR(nc.ip.c_str(), ip_addr, "Ip address should match");
            TEST_MEM(nc.buffer, nc.size, udp_server_packet_1, sizeof(udp_server_packet_1), "First packet buffer should match");
            break;
        }
        case 2:
        {
            TEST_EQ(nc.socket_type, NetworkInspector::SocketType::SocketUDP, "Socket should be of SocketUDP type");
            TEST_EQ(nc.type,
                    NetworkInspector::ContextType::SendToCallback,
                    "Second packet received should be of type ContextType::SendToCallback");
            TEST_EQ(nc.port, port, "The port number should match");
            TEST_STR(nc.ip.c_str(), ip_addr, "Ip address should match");
            TEST_MEM(nc.buffer, nc.size, udp_client_packet_1, sizeof(udp_client_packet_1), "First packet buffer should match");
            nc.buffer = (unsigned char*)udp_modified_client_packet_1;
            nc.size   = sizeof(udp_modified_client_packet_1);
            break;
        }
        case 3:
        {
            TEST_EQ(nc.socket_type, NetworkInspector::SocketType::SocketUDP, "Socket should be of SocketUDP type");
            TEST_EQ(nc.type,
                    NetworkInspector::ContextType::RecvFromCallback,
                    "Second packet received should be of type ContextType::RecvFromCallback");
            TEST_EQ(nc.port, port, "The port number should match");
            TEST_STR(nc.ip.c_str(), ip_addr, "Ip address should match");
            TEST_MEM(nc.buffer, nc.size, udp_modified_server_packet_1, sizeof(udp_modified_server_packet_1), "First packet buffer should match");
            memcpy(nc.buffer, udp_modified_client_packet_1, min(sizeof(udp_modified_client_packet_1), nc.recv_buffer_size));
            nc.size = sizeof(udp_modified_client_packet_1);
            break;
        }
        case 4:
        {
            TEST_EQ(nc.socket_type, NetworkInspector::SocketType::SocketUDP, "Socket should be of SocketUDP type");
            TEST_EQ(nc.type, NetworkInspector::ContextType::SendToCallback, "Packet sent should be of type ContextType::SendToCallback");
            TEST_EQ(nc.port, port, "The port number should match");
            TEST_STR(nc.ip.c_str(), ip_addr, "Ip address should match");
            TEST_MEM(nc.buffer, nc.size, udp_client_packet_2, sizeof(udp_client_packet_2), "Packet buffer should match");
            break;
        }
        case 5:
        {
            TEST_EQ(nc.socket_type, NetworkInspector::SocketType::SocketUDP, "Socket should be of SocketUDP type");
            TEST_EQ(nc.type,
                    NetworkInspector::ContextType::RecvFromCallback,
                    "Packet received should be of type ContextType::RecvFromCallback");
            TEST_EQ(nc.port, port, "The port number should match");
            TEST_STR(nc.ip.c_str(), ip_addr, "Ip address should match");
            TEST_MEM(nc.buffer, nc.size, udp_server_packet_2, sizeof(udp_server_packet_2), "Packet buffer should match");
            break;
        }
        case 6:
        {
            TEST_EQ(nc.socket_type, NetworkInspector::SocketType::SocketUDP, "Socket should be of SocketUDP type");
            TEST_EQ(nc.type, NetworkInspector::ContextType::SendToCallback, "Packet sent should be of type ContextType::SendToCallback");
            TEST_EQ(nc.port, port, "The port number should match");
            TEST_STR(nc.ip.c_str(), ip_addr, "Ip address should match");
            TEST_MEM(nc.buffer, nc.size, udp_client_packet_3, sizeof(udp_client_packet_3), "Packet buffer should match");
            break;
        }
        case 7:
        {
            TEST_EQ(nc.socket_type, NetworkInspector::SocketType::SocketUDP, "Socket should be of SocketUDP type");
            TEST_EQ(nc.type,
                    NetworkInspector::ContextType::RecvFromCallback,
                    "Packet received should be of type ContextType::RecvFromCallback");
            TEST_EQ(nc.port, port, "The port number should match");
            TEST_STR(nc.ip.c_str(), ip_addr, "Ip address should match");
            TEST_MEM(nc.buffer, nc.size, udp_server_packet_3, sizeof(udp_server_packet_3), "Packet buffer should match");
            break;
        }
        default:
            TEST_EQ(0, 1, "The callback was called too many times");
            break;
    }
    test_number++;
}

void StartNetworkInspector()
{
    NetworkInspector::Init();
    NetworkInspector::RegisterCallback(NetworkInspectorCallback);
    NetworkInspector::Start();
};

void StopNetworkInspector()
{
    NetworkInspector::Stop();
    NetworkInspector::Cleanup();
}

int main()
{
    WSAData wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        puts("Error: WSAStartup failed");
        return -1;
    }

    StartNetworkInspector();

    SOCKET s;
    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == INVALID_SOCKET)
    {
        puts("Error: socket failed");
        WSACleanup();
        return -1;
    }

    sockaddr_in addr_to;
    memset((void*)&addr_to, 0, sizeof(addr_to));
    addr_to.sin_family      = AF_INET;
    addr_to.sin_addr.s_addr = inet_addr(ip_addr);
    addr_to.sin_port        = htons(port);

    char recv_buffer[0x100];
    int nb_read;
    sockaddr addr_from;
    int len;

    // Test normal operation
    memset(recv_buffer, 0, sizeof(recv_buffer));
    memset(&addr_from, 0, sizeof(addr_from));
    len = sizeof(addr_from);
    sendto(s, udp_client_packet_1, sizeof(udp_client_packet_1), 0, (sockaddr*)&addr_to, sizeof(addr_to));
    recvfrom(s, recv_buffer, sizeof(recv_buffer) - 1, 0, &addr_from, &len);

    // Test modified received packet
    memset(recv_buffer, 0, sizeof(recv_buffer));
    memset(&addr_from, 0, sizeof(addr_from));
    len = sizeof(addr_from);
    sendto(s, udp_client_packet_1, sizeof(udp_client_packet_1), 0, (sockaddr*)&addr_to, sizeof(addr_to));
    nb_read = recvfrom(s, recv_buffer, sizeof(recv_buffer) - 1, 0, &addr_from, &len);

    TEST_NEQ(nb_read, SOCKET_ERROR, "Recv should not fail");
    if (nb_read != SOCKET_ERROR)
    {
        TEST_MEM(recv_buffer,
                 nb_read,
                 udp_modified_client_packet_1,
                 sizeof(udp_modified_client_packet_1),
                 "The packet received packet should be modified by the callback");
    }

    // Test IP address exclusion
    NetworkInspector::AddIpAddressExclusion(ip_addr);
    memset(recv_buffer, 0, sizeof(recv_buffer));
    memset(&addr_from, 0, sizeof(addr_from));
    len = sizeof(addr_from);
    sendto(s, udp_excluded_client_packet_1, sizeof(udp_excluded_client_packet_1), 0, (sockaddr*)&addr_to, sizeof(addr_to));
    nb_read = recvfrom(s, recv_buffer, sizeof(recv_buffer) - 1, 0, &addr_from, &len);

    TEST_NEQ(nb_read, SOCKET_ERROR, "Recv should not fail");
    if (nb_read != SOCKET_ERROR)
    {
        TEST_MEM(recv_buffer,
                 nb_read,
                 udp_excluded_server_packet_1,
                 sizeof(udp_excluded_server_packet_1),
                 "Should receive the IP address exclusion packet");
    }
    // Test remove IP address exclusion
    NetworkInspector::RemoveIpAddressExclusion(ip_addr);
    memset(recv_buffer, 0, sizeof(recv_buffer));
    memset(&addr_from, 0, sizeof(addr_from));
    len = sizeof(addr_from);
    sendto(s, udp_client_packet_2, sizeof(udp_client_packet_2), 0, (sockaddr*)&addr_to, sizeof(addr_to));
    recvfrom(s, recv_buffer, sizeof(recv_buffer) - 1, 0, &addr_from, &len);

    // Test Port-based exclusion
    NetworkInspector::AddPortExclusion(port);
    memset(recv_buffer, 0, sizeof(recv_buffer));
    memset(&addr_from, 0, sizeof(addr_from));
    len = sizeof(addr_from);
    sendto(s, udp_excluded_client_packet_2, sizeof(udp_excluded_client_packet_2), 0, (sockaddr*)&addr_to, sizeof(addr_to));
    nb_read = recvfrom(s, recv_buffer, sizeof(recv_buffer) - 1, 0, &addr_from, &len);

    TEST_NEQ(nb_read, SOCKET_ERROR, "Recv should not fail");
    if (nb_read != SOCKET_ERROR)
    {
        TEST_MEM(recv_buffer,
                 nb_read,
                 udp_excluded_server_packet_2,
                 sizeof(udp_excluded_server_packet_2),
                 "Should receive the port number exclusion packet");
    }

    // Test Remove port-based exclusion
    NetworkInspector::RemovePortExclusion(port);
    memset(recv_buffer, 0, sizeof(recv_buffer));
    memset(&addr_from, 0, sizeof(addr_from));
    len = sizeof(addr_from);
    sendto(s, udp_client_packet_3, sizeof(udp_client_packet_3), 0, (sockaddr*)&addr_to, sizeof(addr_to));
    recvfrom(s, recv_buffer, sizeof(recv_buffer) - 1, 0, &addr_from, &len);

    TEST_EQ(test_number, 8, "The number of NetworkInspector callbacks should match");
    PRINT_TEST_RESULT();

    StopNetworkInspector();
    closesocket(s);
    WSACleanup();
    return 0;
}
