#ifndef NETWORK_INSPECTOR_UTILS_H_
#define NETWORK_INSPECTOR_UTILS_H_

#include <cstdint>

int ProcessGetBacktrace(uintptr_t rip, uintptr_t rsp, uintptr_t* backtrace, int nb_frame);

#endif // NETWORK_INSPECTOR_UTILS_H_
