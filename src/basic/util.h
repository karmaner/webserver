#ifndef __SRC_BASIC_UTIL_H__
#define __SRC_BASIC_UTIL_H__
#include <cxxabi.h>
#include <iostream>
#include <stdint.h>
#include <string>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <iomanip>
#include <boost/lexical_cast.hpp>

#include "src/util/hash_util.h"

namespace webserver {
pid_t GetThreadId();
uint64_t GetFiberId();
uint64_t GetElapseMS();
std::string GetThreadName();
void SetThreadName(const std::string &name);
uint64_t GetCurrentMS();
uint64_t GetCurrentUS();

void Backtrace(std::vector<std::string>& bt, int size, int skip = 1);
std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");

std::string Time2Str(time_t ts = time(0), const std::string& format = "%Y-%m-%d %H:%M:%S");

class FSUtil {
public:
    static void ListAllFile(std::vector<std::string>& files
                            ,const std::string& path
                            ,const std::string& subfix);
    static bool Mkdir(const std::string& dirname);
    static bool IsRunningPidfile(const std::string& pidfile);
    static bool Rm(const std::string& path);
    static bool Mv(const std::string& from, const std::string& to);
    static bool Realpath(const std::string& path, std::string& rpath);
    static bool Symlink(const std::string& frm, const std::string& to);
    static bool Unlink(const std::string& filename, bool exist = false);
    static std::string Dirname(const std::string& filename);
    static std::string Basename(const std::string& filename);
    static bool OpenForRead(std::ifstream& ifs, const std::string& filename
                    ,std::ios_base::openmode mode = std::ios_base::in);
    static bool OpenForWrite(std::ofstream& ofs, const std::string& filename
                    ,std::ios_base::openmode mode = std::ios_base::out);
};

template<class Map, class K, class V>
V GetParamValue(const Map& m, const K& k, const V& def = V()) {
    auto it = m.find(k);
    if(it == m.end()) {
        return def;
    }
    try {
        return boost::lexical_cast<V>(it->second);
    } catch (...) {
    }
    return def;
}

template<class Map, class K, class V>
bool CheckGetParamValue(const Map& m, const K& k, V& v) {
    auto it = m.find(k);
    if(it == m.end()) {
        return false;
    }
    try {
        v = boost::lexical_cast<V>(it->second);
        return true;
    } catch (...) {
    }
    return false;
}

} // namespace webserver

#endif