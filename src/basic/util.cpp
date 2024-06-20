#include "util.h"

namespace webserver {
    pid_t GetThreadId() {
        return syscall(SYS_gettid);
    }

    uint64_t GetFiberId() {
        //TODO: 协程未实现先返回0
        return 0;
    }

    uint64_t GetElapseMS() {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        // 将 timespec 结构体转换为 uint64_t 类型
        return (uint64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;
    }

    std::string GetThreadName() {
        char thread_name[16] = {0};
        pthread_getname_np(pthread_self(), thread_name, 16);
        return std::string(thread_name);
    }

    void SetThreadName(const std::string& name) {
        pthread_setname_np(pthread_self(), name.substr(0, 15).c_str());
    }

    uint64_t GetCurrentMS() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (uint64_t)tv.tv_sec * 1000ul + tv.tv_usec / 1000;
    } 

    uint64_t GetCurrentUS() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (uint64_t)tv.tv_sec * 1000 * 1000ul + tv.tv_usec;
    }

}