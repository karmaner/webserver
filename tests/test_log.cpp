#include "src/basic/log.h"
#include "src/basic/util.h"
#include <iostream>

int main(int argc, char **argv)
{
    webserver::Logger::ptr logger(new webserver::Logger);
    logger->addAppender(webserver::LogAppender::ptr(new webserver::StdoutLogAppender));
    webserver::Logger::ptr t_logger = WEBSERVER_LOG_NAME("system");

    webserver::FileLogAppender::ptr file_appender(new webserver::FileLogAppender("./bin/log.txt"));
    webserver::LogFormatter::ptr fmt(new webserver::LogFormatter("%d%T%p%T%m%n"));
    file_appender->setFormatter(fmt);
    file_appender->setLevel(webserver::LogLevel::ERROR);

    logger->addAppender(file_appender);
    t_logger->addAppender(webserver::LogAppender::ptr(new webserver::StdoutLogAppender));
    // webserver::LogEvent::ptr event(new webserver::LogEvent(__FILE__, __LINE__, 0, webserver::GetThreadId(),
    // webserver::GetFiberId(), time(0))); event->getSS() << "hello webserver log";
    // logger->log(webserver::LogLevel::DEBUG, event);
    std::cout << "hello webserver log" << std::endl;

    WEBSERVER_LOG_INFO(logger) << "test macro";
    WEBSERVER_LOG_ERROR(logger) << "test macro error";

    WEBSERVER_LOG_FMT_ERROR(logger, "test macro fmt error %s", "aa");

    auto l = webserver::LoggerMgr::GetInstance()->getLogger("xx");
    WEBSERVER_LOG_INFO(l) << "xxx";
    WEBSERVER_LOG_INFO(t_logger) << "good?";
    return 0;
}