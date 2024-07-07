#include "src/webserver.h"
#include <unistd.h>

webserver::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();


void func1() {
    WEBSERVER_LOG_INFO(g_logger) << "name: " << webserver::Thread::GetName()
                                << " this.name: " << webserver::Thread::GetThis()->getName()
                                << " id:" << webserver::GetThreadId()
                                << " this.id:" << webserver::Thread::GetThis()->getId();
}

void func2() {

}

int main(int argc, char* argv[]) {

    std::vector<webserver::Thread::ptr> thrs;
    for(int i = 0; i < 5; ++i) {
        webserver::Thread::ptr thr(new webserver::Thread(&func1, "name_" + std::to_string(i)));
        thrs.push_back(thr);
    }

    sleep(20);
    for(int i = 0; i < 5; ++i) {
        thrs[i]->join();
    }
    WEBSERVER_LOG_INFO(g_logger) << "Thread test end";
    return 0;
}