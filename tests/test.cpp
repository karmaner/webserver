#include <iostream>
#include "../src/basic/log.h"


int main(int argc, char* argv[]) {
    webserver::Logger::ptr logger(new webserver::Logger);
    logger->addAppender(webserver::LogAppender::ptr (new webserver::FileLogAppender("/home/karmaner/webserver/bin/log.txt")));
    logger->addAppender(webserver::LogAppender::ptr (new webserver::StdoutLogAppender()));
    
    // logger->addAppender(webserver::LogAppender::ptr (new webserver::StdoutLogAppender));

    webserver::LogEvent::ptr event(new webserver::LogEvent("Debug日志", webserver::LogLevel::DEBUG, __FILE__, __LINE__, 0, 1, 2,time(0), "线程01"));

    

    logger->log(event);
    logger->info(event);
    std::cout << "Hello webserver log" << std::endl;

    return 0;
}