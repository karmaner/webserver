#include <iostream>
#include "../src/basic/log.h"
#include "../src/basic/util.h"


int main(int argc, char* argv[]) {
    webserver::Logger::ptr logger(new webserver::Logger);
    logger->addAppender(webserver::LogAppender::ptr (new webserver::FileLogAppender("/home/karmaner/webserver/bin/log.txt")));
    logger->addAppender(webserver::LogAppender::ptr (new webserver::StdoutLogAppender()));
    
    // logger->addAppender(webserver::LogAppender::ptr (new webserver::StdoutLogAppender));

    // webserver::LogEvent::ptr event(new webserver::LogEvent("Debug日志", webserver::LogLevel::DEBUG, __FILE__, __LINE__, 0, 
    //                                                            webserver::GetThreadId(), webserver::GetFiberId(),time(0), webserver::GetThreadName()));

    // event->getSS() << "Hello World!";

    logger->setLevel(webserver::LogLevel::FATAL);

    WEBSERVER_LOG_FATAL(logger) << "FATAL show up";

    WEBSERVER_LOG_DEBUG(logger) << "This is Debug info";

    //TODO: 有问题
    //WEBSERVER_LOG_FMT_FATAL(logger, "fatal %s:%d:%s;%s", "你好！", __FILE__, __LINE__);

    // logger->log(event);
    // logger->info(event);

    // webserver::Logger::ptr l2 =  WEBSERVER_LOG_ROOT();

    webserver::Logger::ptr l2 = WEBSERVER_LOG_NAME("DBA日志");
    l2->addAppender(webserver::LogAppender::ptr (new webserver::StdoutLogAppender()));

    WEBSERVER_LOG_DEBUG(l2) << "LoggerMgr test";    

    webserver::LoggerManager l;
    l.getLogger("test日志")->addAppender(webserver::LogAppender::ptr (new webserver::StdoutLogAppender()));
    WEBSERVER_LOG_DEBUG(l.getLogger("test日志")) << "DEBUG show up";

    return 0;
}