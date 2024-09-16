#include "src/basic/env.h"
#include <unistd.h>
#include <iostream>
#include <fstream>

struct A {
    A() {
        std::ifstream ifs("/proc/" + std::to_string(getpid()) + "/cmdline", std::ios::binary);
        std::string content;
        content.resize(4096);

        ifs.read(&content[0], content.size());
        content.resize(ifs.gcount());

        for(size_t i = 0; i < content.size(); ++i) {
            std::cout << i << " - " << content[i] << " - " << (int)content[i] << std::endl;
        }
    }
};

A a;

int main(int argc, char* argv[]) {
    std::cout << "argc=" << argc << std::endl;
    webserver::EnvMgr::GetInstance()->addHelp("s", "start with the terminal");
    webserver::EnvMgr::GetInstance()->addHelp("d", "run as daemon");
    webserver::EnvMgr::GetInstance()->addHelp("p", "print help");
    if(!webserver::EnvMgr::GetInstance()->init(argc, argv)) {
        webserver::EnvMgr::GetInstance()->printHelp();
        return 0;
    }

    std::cout << "exe=" << webserver::EnvMgr::GetInstance()->getExe() << std::endl;
    std::cout << "cwd=" << webserver::EnvMgr::GetInstance()->getCwd() << std::endl;

    std::cout << "path=" << webserver::EnvMgr::GetInstance()->getEnv("PATH", "xxx") << std::endl;
    std::cout << "test=" << webserver::EnvMgr::GetInstance()->getEnv("TEST", "") << std::endl;
    std::cout << "set env " << webserver::EnvMgr::GetInstance()->setEnv("TEST", "yy") << std::endl;
    std::cout << "test=" << webserver::EnvMgr::GetInstance()->getEnv("TEST", "") << std::endl;
    if(webserver::EnvMgr::GetInstance()->has("p")) {
        webserver::EnvMgr::GetInstance()->printHelp();
    }
    return 0;
}
