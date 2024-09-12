#include "src/basic/daemon.h"
#include "src/basic/iomanager.h"
#include "src/basic/log.h"

static webserver::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();

webserver::Timer::ptr timer;
int server_main(int argc, char** argv) {
    WEBSERVER_LOG_INFO(g_logger) << webserver::ProcessInfoMgr::GetInstance()->toString();
    webserver::IOManager iom(1);
    timer = iom.addTimer(1000, [](){
            WEBSERVER_LOG_INFO(g_logger) << "onTimer";
            static int count = 0;
            if(++count > 10) {
                exit(1);
            }
    }, true);
    return 0;
}

int main(int argc, char** argv) {
    return webserver::start_daemon(argc, argv, server_main, argc != 1);
}
