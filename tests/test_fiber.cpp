#include "src/webserver.h"

webserver::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();


void run_in_fiber() {
    WEBSERVER_LOG_INFO(g_logger) << "run_in_fiber begin";
    webserver::Fiber::GetThis() -> swapOut();
    WEBSERVER_LOG_INFO(g_logger) << "run_in_fiber end";
}

void test_fiber() {

    WEBSERVER_LOG_INFO(g_logger) << "main begin -1";
    {
        webserver::Fiber::GetThis();
        WEBSERVER_LOG_INFO(g_logger) << "main begin";
        webserver::Fiber::ptr fiber(new webserver::Fiber(run_in_fiber));

        fiber->swapIn();
        WEBSERVER_LOG_INFO(g_logger) << "main afer swapIn";
        fiber->swapIn();
        WEBSERVER_LOG_INFO(g_logger) << "main end";
    }

    WEBSERVER_LOG_INFO(g_logger) << "main after end2";
}

int main(int argc, char* argv[]) {

    webserver::Thread::SetName("main");

    std::vector<webserver::Thread::ptr> thrs;
    for (int i = 0; i < 3; ++i) {
        thrs.push_back(webserver::Thread::ptr(new webserver::Thread(&test_fiber, "name" + std::to_string(i))));
    }

    for (auto i : thrs) {
        i->join();
    }
    return 0;
}

