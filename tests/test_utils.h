#ifndef NETWORK_INSPECTOR_TEST_UTILS_H_
#define NETWORK_INSPECTOR_TEST_UTILS_H_

#include <algorithm>
#include <cassert>
#include <cstdio>

// Note: Client and Server packets should have different size
const char udp_client_packet_1[] = "Hello from UDP Client";
const char udp_server_packet_1[] = "Hello from UDP Server---";

const char udp_client_packet_2[] = "Hello from UDP Client 2";
const char udp_server_packet_2[] = "Hello from UDP Server--- 2";

const char udp_client_packet_3[] = "Hello from UDP Client 3";
const char udp_server_packet_3[] = "Hello from UDP Server--- 3";

const char udp_modified_client_packet_1[] = "Hello from UDP Client patched";
const char udp_modified_server_packet_1[] = "Hello from UDP Server--- patched";

const char tcp_client_packet_1[] = "Hello from TCP Client 1";
const char tcp_server_packet_1[] = "Hello from TCP Server--- 1";

const char tcp_client_packet_2[] = "Hello from TCP Client 2";
const char tcp_server_packet_2[] = "Hello from TCP Server--- 2";

const char tcp_client_packet_3[] = "Hello from TCP Client 3";
const char tcp_server_packet_3[] = "Hello from TCP Server--- 3";

const char tcp_modified_client_packet_1[] = "Hello from TCP Client patched";
const char tcp_modified_server_packet_1[] = "Hello from TCP Server--- patched";

const char tcp_excluded_client_packet_1[] = "Hello from TCP Client excluded by IP address";
const char tcp_excluded_server_packet_1[] = "Hello from TCP Server--- excluded by IP address";

const char tcp_excluded_client_packet_2[] = "Hello from TCP Client excluded by port number";
const char tcp_excluded_server_packet_2[] = "Hello from TCP Server--- excluded by port number";

const char udp_excluded_client_packet_1[] = "Hello from UDP Client excluded by IP address";
const char udp_excluded_server_packet_1[] = "Hello from UDP Server--- excluded by IP address";

const char udp_excluded_client_packet_2[] = "Hello from UDP Client excluded by port number";
const char udp_excluded_server_packet_2[] = "Hello from UDP Server--- excluded by port number";

extern int kNbTestInputSuccess = 0;
extern int kNbTestInputFailed  = 0;

#define TEST_EQ(val1, val2, message) \
    if (val1 != val2)                \
    {                                \
        FAIL_MESSAGE(message);       \
        kNbTestInputFailed++;        \
    }                                \
    else                             \
    {                                \
        kNbTestInputSuccess++;       \
    }

#define TEST_NEQ(val1, val2, message) \
    if (val1 == val2)                 \
    {                                 \
        FAIL_MESSAGE(message);        \
        kNbTestInputFailed++;         \
    }                                 \
    else                              \
    {                                 \
        kNbTestInputSuccess++;        \
    }

#define TEST_MEM(mem1, size1, mem2, size2, message)                            \
    if (size1 != size2 || memcmp((void*)mem1, (void*)mem2, min(size1, size2))) \
    {                                                                          \
        FAIL_MESSAGE(message);                                                 \
        kNbTestInputFailed++;                                                  \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        kNbTestInputSuccess++;                                                 \
    }

#define TEST_STR(str1, str2, message) \
    if (strcmp(str1, str2))           \
    {                                 \
        FAIL_MESSAGE(message);        \
        kNbTestInputFailed++;         \
    }                                 \
    else                              \
    {                                 \
        kNbTestInputSuccess++;        \
    }
#define FAIL_MESSAGE(msg) fprintf(stderr, "Test failed: " msg " in %s:%d\n", __FILE__, __LINE__);

#define PRINT_TEST_RESULT()                                                                                           \
    if (kNbTestInputFailed != 0)                                                                                      \
    {                                                                                                                 \
        printf("FAIL: %d test(s) failed\n", kNbTestInputFailed);                                                      \
        printf("%d/%d tests passed\n", kNbTestInputSuccess, kNbTestInputSuccess + kNbTestInputFailed);                \
    }                                                                                                                 \
    else                                                                                                              \
    {                                                                                                                 \
        printf("SUCCESS: All tests passed (%d/%d)\n", kNbTestInputSuccess, kNbTestInputSuccess + kNbTestInputFailed); \
    }

#endif // NETWORK_INSPECTOR_TEST_UTILS_H_
