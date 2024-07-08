#ifndef __SRC__UTIL_H__
#define __SRC__UTIL_H__
#include <cxxabi.h>
#include <iostream>
#include <stdint.h>
#include <string>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

namespace webserver {
pid_t GetThreadId();
uint64_t GetFiberId();
uint64_t GetElapseMS();
std::string GetThreadName();
void SetThreadName(const std::string &name);
uint64_t GetCurrentMS();
uint64_t GetCurrentUS();
} // namespace webserver

#endif