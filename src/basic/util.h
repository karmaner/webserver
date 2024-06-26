#ifndef __SRC__UTIL_H__
#define __SRC__UTIL_H__
#include <sys/types.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <cxxabi.h>
#include <string>
#include <iostream>

namespace webserver {
    pid_t GetThreadId();
    uint64_t GetFiberId();
    uint64_t GetElapseMS();
    std::string GetThreadName();
    void SetThreadName(const std::string& name);
    uint64_t GetCurrentMS();
    uint64_t GetCurrentUS();
}

#endif