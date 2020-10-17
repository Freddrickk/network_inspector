#include "network_inspector/network_inspector.h"
#include "test_utils.h"

#include <Windows.h>
#include <cstdio>

constexpr char* ip_addr       = "127.0.0.1";
constexpr unsigned short port = 31337;
size_t test_number            = 0;

void NetworkInspectorCallback(NetworkInspector::NetworkContext& nc)
{
    switch (test_number)
    {
        case 0:
        {
            TEST_EQ(nc.socket_type, NetworkInspector::SocketType::SocketTCP, "Socket should be of SocketTCP type");
            TEST_EQ(nc.type, NetworkInspector::ContextType::RecvCallback, "First packet sent should be of type ContextType::RecvCallback");
            TEST_EQ(nc.port, port, "The port number should match");
            TEST_STR(nc.ip.c_str(), ip_addr, "Ip address should match");
            TEST_MEM(nc.buffer, nc.size, tcp_client_packet_1, sizeof(tcp_client_packet_1), "First packet buffer should match");
            break;
        }
        case 1:
        {
            TEST_EQ(nc.socket_type, NetworkInspector::SocketType::SocketTCP, "Socket should be of SocketTCP type");
            TEST_EQ(nc.type, NetworkInspector::ContextType::SendCallback, "First packet received should be of type ContextType::SendCallback");
            TEST_EQ(nc.port, port, "The port number should match");
            TEST_STR(nc.ip.c_str(), ip_addr, "Ip address should match");
            TEST_MEM(nc.buffer, nc.size, tcp_server_packet_1, sizeof(tcp_server_packet_1), "First packet buffer should match");
            break;
        }
        case 2:
        {
            TEST_EQ(nc.socket_type, NetworkInspector::SocketType::SocketTCP, "Socket should be of SocketTCP type");
            TEST_EQ(nc.type, NetworkInspector::ContextType::RecvCallback, "Second packet received should be of type ContextType::RecvCallback");
            TEST_EQ(nc.port, port, "The port number should match");
            TEST_STR(nc.ip.c_str(), ip_addr, "Ip address should match");
            TEST_MEM(nc.buffer, nc.size, tcp_modified_client_packet_1, sizeof(tcp_modified_client_packet_1), "First packet buffer should match");
            memcpy(nc.buffer, tcp_modified_server_packet_1, min(sizeof(tcp_modified_server_packet_1), nc.recv_buffer_size));
            nc.size = sizeof(tcp_modified_server_packet_1);
            break;
        }
        case 3:
        {
            TEST_EQ(nc.socket_type, NetworkInspector::SocketType::SocketTCP, "Socket should be of SocketTCP type");
            TEST_EQ(nc.type, NetworkInspector::ContextType::SendCallback, "Second packet received should be of type ContextType::SendCallback");
            TEST_EQ(nc.port, port, "The port number should match");
            TEST_STR(nc.ip.c_str(), ip_addr, "Ip address should match");
            TEST_MEM(nc.buffer, nc.size, tcp_server_packet_1, sizeof(tcp_server_packet_1), "First packet buffer should match");
            nc.buffer = (unsigned char*)tcp_modified_server_packet_1;
            nc.size   = sizeof(tcp_modified_server_packet_1);
            break;
        }
        case 4:
        {
            TEST_EQ(nc.socket_type, NetworkInspector::SocketType::SocketTCP, "Socket should be of SocketTCP type");
            TEST_EQ(nc.type, NetworkInspector::ContextType::RecvCallback, "Packet sent should be of type ContextType::RecvCallback");
            TEST_EQ(nc.port, port, "The port number should match");
            TEST_STR(nc.ip.c_str(), ip_addr, "Ip address should match");
            TEST_MEM(nc.buffer, nc.size, tcp_client_packet_2, sizeof(tcp_client_packet_2), "Packet buffer should match");
            break;
        }
        case 5:
        {
            TEST_EQ(nc.socket_type, NetworkInspector::SocketType::SocketTCP, "Socket should be of SocketTCP type");
            TEST_EQ(nc.type, NetworkInspector::ContextType::SendCallback, "First packet received should be of type ContextType::SendCallback");
            TEST_EQ(nc.port, port, "The port number should match");
            TEST_STR(nc.ip.c_str(), ip_addr, "Ip address should match");
            TEST_MEM(nc.buffer, nc.size, tcp_server_packet_2, sizeof(tcp_server_packet_2), "Packet buffer should match");
            break;
        }
        case 6:
        {
            TEST_EQ(nc.socket_type, NetworkInspector::SocketType::SocketTCP, "Socket should be of SocketTCP type");
            TEST_EQ(nc.type, NetworkInspector::ContextType::RecvCallback, "First packet sent should be of type ContextType::RecvCallback");
            TEST_EQ(nc.port, port, "The port number should match");
            TEST_STR(nc.ip.c_str(), ip_addr, "Ip address should match");
            TEST_MEM(nc.buffer, nc.size, tcp_client_packet_3, sizeof(tcp_client_packet_3), "Packet buffer should match");
            break;
        }
        case 7:
        {
            TEST_EQ(nc.socket_type, NetworkInspector::SocketType::SocketTCP, "Socket should be of SocketTCP type");
            TEST_EQ(nc.type, NetworkInspector::ContextType::SendCallback, "First packet received should be of type ContextType::SendCallback");
            TEST_EQ(nc.port, port, "The port number should match");
            TEST_STR(nc.ip.c_str(), ip_addr, "Ip address should match");
            TEST_MEM(nc.buffer, nc.size, tcp_server_packet_3, sizeof(tcp_server_packet_3), "First packet buffer should match");
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

void WaitForSend(SOCKET s)
{
    fd_set fds;
    fds.fd_count    = 1;
    fds.fd_array[0] = s;
    select(0, nullptr, &fds, nullptr, nullptr);
}

void WaitForRecv(SOCKET s)
{
    fd_set fds;
    fds.fd_count    = 1;
    fds.fd_array[0] = s;
    select(0, &fds, nullptr, nullptr, nullptr);
}

SOCKET WaitForConnection(SOCKET s)
{
    listen(s, SOMAXCONN);
    return accept(s, nullptr, nullptr);
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
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET)
    {
        puts("Error: socket failed");
        WSACleanup();
        return -1;
    }
    int optval = 1;
    //setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));

    sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip_addr);
    addr.sin_port        = htons(port);


    if (bind(s, (sockaddr*)&addr, sizeof(addr)) != 0)
    {
        puts("Error: bind failed");
        printf("WSAGetLastError: %d\n", WSAGetLastError());
        closesocket(s);
        WSACleanup();
        return -1;
    }

    SOCKET conn = WaitForConnection(s);

    char recv_buffer[0x100];
    int nb_read = 0;

    // Test normal operation
    WaitForRecv(conn);
    memset(recv_buffer, 0, sizeof(recv_buffer));
    recv(conn, recv_buffer, sizeof(recv_buffer) - 1, 0);
    send(conn, tcp_server_packet_1, sizeof(tcp_server_packet_1), 0);

    // Test modified received packet
    WaitForRecv(conn);
    memset(recv_buffer, 0, sizeof(recv_buffer));
    nb_read = recv(conn, recv_buffer, sizeof(recv_buffer) - 1, 0);
    send(conn, tcp_server_packet_1, sizeof(tcp_server_packet_1), 0);

    TEST_NEQ(nb_read, SOCKET_ERROR, "Recv should not fail");
    if (nb_read != SOCKET_ERROR)
    {
        TEST_MEM(recv_buffer,
                 nb_read,
                 tcp_modified_server_packet_1,
                 sizeof(tcp_modified_server_packet_1),
                 "The packet received packet should be modified by the callback");
    }


    // Test IP address exclusion
    NetworkInspector::AddIpAddressExclusion(ip_addr);
    WaitForRecv(conn);
    memset(recv_buffer, 0, sizeof(recv_buffer));
    nb_read = recv(conn, recv_buffer, sizeof(recv_buffer) - 1, 0);
    send(conn, tcp_excluded_server_packet_1, sizeof(tcp_excluded_server_packet_1), 0);

    TEST_NEQ(nb_read, SOCKET_ERROR, "Recv should not fail");
    if (nb_read != SOCKET_ERROR)
    {
        TEST_MEM(recv_buffer,
                 nb_read,
                 tcp_excluded_client_packet_1,
                 sizeof(tcp_excluded_client_packet_1),
                 "Should receive the IP address exclusion packet");
    }

    // Test remove IP address exclusion
    NetworkInspector::RemoveIpAddressExclusion(ip_addr);
    WaitForRecv(conn);
    memset(recv_buffer, 0, sizeof(recv_buffer));
    recv(conn, recv_buffer, sizeof(recv_buffer) - 1, 0);
    send(conn, tcp_server_packet_2, sizeof(tcp_server_packet_2), 0);

    // Test Port-based exclusion
    NetworkInspector::AddPortExclusion(port);
    WaitForRecv(conn);
    memset(recv_buffer, 0, sizeof(recv_buffer));
    nb_read = recv(conn, recv_buffer, sizeof(recv_buffer) - 1, 0);
    send(conn, tcp_excluded_server_packet_2, sizeof(tcp_excluded_server_packet_2), 0);

    TEST_NEQ(nb_read, SOCKET_ERROR, "Recv should not fail");
    if (nb_read != SOCKET_ERROR)
    {
        TEST_MEM(recv_buffer,
                 nb_read,
                 tcp_excluded_client_packet_2,
                 sizeof(tcp_excluded_client_packet_2),
                 "Should receive the port number exclusion packet");
    }

    // Test Remove port-based exclusion
    NetworkInspector::RemovePortExclusion(port);
    WaitForRecv(conn);
    memset(recv_buffer, 0, sizeof(recv_buffer));
    recv(conn, recv_buffer, sizeof(recv_buffer) - 1, 0);
    send(conn, tcp_server_packet_3, sizeof(tcp_server_packet_3), 0);

    TEST_EQ(test_number, 8, "The number of NetworkInspector callbacks should match");
    PRINT_TEST_RESULT();

    StopNetworkInspector();
    closesocket(conn);
    closesocket(s);
    WSACleanup();
    return 0;
}
