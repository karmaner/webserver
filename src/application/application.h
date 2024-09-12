#ifndef __SRC_APPLICATION_H__
#define __SRC_APPLICATION_H__

#include "src/http/http_server.h"

namespace webserver {

class Application {
public:
    Application();

    static Application* GetInstance() { return s_instance;}
    bool init(int argc, char** argv);
    bool run();
private:
    int main(int argc, char** argv);
    int run_fiber();
private:
    int m_argc = 0;
    char** m_argv = nullptr;

    std::vector<webserver::http::HttpServer::ptr> m_httpservers;
    static Application* s_instance;
};

}

#endif