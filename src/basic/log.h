#ifndef __SRC_LOG_H__
#define __SRC_LOG_H__
#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <vector>
#include <chrono>
#include <stdarg.h>
#include <map>
#include "util.h"
#include "singleton.h"


#define WEBSERVER_LOG_ROOT() webserver::LoggerMgr::GetInstance()->getRoot()

#define WEBSERVER_LOG_NAME(name) webserver::LoggerMgr::GetInstance()->getLogger(name)

#define WEBSERVER_LOG_LEVEL(logger, level) \
    if(logger->getLevel() <= level) \
        webserver::LogEventWrap(logger, webserver::LogEvent::ptr(new webserver::LogEvent(logger->getName(), level, __FILE__, __LINE__, webserver::GetElapseMS(), \
                                                                    webserver::GetThreadId(), webserver::GetFiberId(),time(0), webserver::GetThreadName()))).getLogEvent()->getSS()

#define WEBSERVER_LOG_FATAL(logger) WEBSERVER_LOG_LEVEL(logger, webserver::LogLevel::FATAL)
#define WEBSERVER_LOG_ERROR(logger) WEBSERVER_LOG_LEVEL(logger, webserver::LogLevel::ERROR)
#define WEBSERVER_LOG_WARN(logger) WEBSERVER_LOG_LEVEL(logger, webserver::LogLevel::WARN)
#define WEBSERVER_LOG_INFO(logger) WEBSERVER_LOG_LEVEL(logger, webserver::LogLevel::INFO)
#define WEBSERVER_LOG_DEBUG(logger) WEBSERVER_LOG_LEVEL(logger, webserver::LogLevel::DEBUG)


#define WEBSERVER_LOG_FMT_LEVEL(logger, level, fmt, ...) \
    if(logger->getLevel() <= level) \
        webserver::LogEventWrap(logger, webserver::LogEvent::ptr(new webserver::LogEvent(logger->getName(), level, __FILE__, __LINE__, webserver::GetElapseMS(), \
                                                                    webserver::GetThreadId(), webserver::GetFiberId(),time(0), webserver::GetThreadName()))).getLogEvent()->printf(fmt, __VA_ARGS__)

#define WEBSERVER_LOG_FMT_FATAL(logger, fmt, ...) WEBSERVER_LOG_FMT_LEVEL(logger, webserver::LogLevel::FATAL, fmt, __VA_ARGS__)
#define WEBSERVER_LOG_FMT_ERROR(logger, fmt, ...) WEBSERVER_LOG_FMT_LEVEL(logger, webserver::LogLevel::ERROR, fmt, __VA_ARGS__)
#define WEBSERVER_LOG_FMT_WARN(logger, fmt, ...) WEBSERVER_LOG_FMT_LEVEL(logger, webserver::LogLevel::WARN, fmt, __VA_ARGS__)
#define WEBSERVER_LOG_FMT_INFO(logger, fmt, ...) WEBSERVER_LOG_FMT_LEVEL(logger, webserver::LogLevel::INFO, fmt, __VA_ARGS__)
#define WEBSERVER_LOG_FMT_DEBUG(logger, fmt, ...) WEBSERVER_LOG_FMT_LEVEL(logger, webserver::LogLevel::DEBUG, fmt, __VA_ARGS__)

namespace webserver { 

class Logger;

// 日志级别
class LogLevel {
public:
    enum Level
    {
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };

    static const char* ToString(LogLevel::Level level);
};

// 日志事件
class LogEvent {
public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent(const std::string &logger_name, LogLevel::Level level, const char *file, int32_t line
            , int64_t elapse, uint32_t thread_id, uint64_t fiber_id, time_t time, const std::string &thread_name);

    const char* getFile() const { return m_file; }                // 文件名
    int32_t getLine() const { return m_line; }                    // 行号
    LogLevel::Level getLevel() { return m_level; }                // 获取日志级别
    uint32_t getElapse() const { return m_elapse; }               // 程序启动到现在的毫秒
    uint32_t getThreadID() const { return m_threadId; }           // 线程id
    uint32_t getFiberId() const { return m_fiberId; }             // 协程id
    std::string getThreadName() { return m_threadName; }          // 线程名
    uint64_t getTime() const { return m_time; }                   // 时间戳
    const std::string& getLoggerName() { return m_loggerName; }   // 获取日志名                    
    const std::string getContent() const { return m_ss.str(); }                                     

    std::stringstream& getSS() { return m_ss; }

    void printf(const char *fmt, ...);

    void vprintf(const char* fmt, va_list ap);

private:
    std::string m_loggerName;
    LogLevel::Level m_level;      // 日志级别
    const char *m_file = nullptr; // 文件名
    int32_t m_line = 0;           // 行号
    uint32_t m_elapse = 0;        // 程序启动到现在的毫秒
    uint32_t m_threadId = 0;      // 线程id
    uint32_t m_fiberId = 0;       // 协程id
    uint64_t m_time;              // 时间戳
    std::string m_threadName;

    std::shared_ptr<Logger> m_logger;
    std::stringstream m_ss;

};




// 日志格式器
class LogFormatter {
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(const std::string& pattern = "%d{%Y-%m-%d %H:%M:%S} [%rms]%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n");

    // %t   %thread_id %m%n
    std::string format(LogEvent::ptr event);
    std::ostream &format(std::ostream &os, LogEvent::ptr event);
public:
    class FormatItem {
    public:
        typedef std::shared_ptr<FormatItem> ptr;
        FormatItem(const std::string& fmt = "") { };
        virtual ~FormatItem() {}
        virtual void format(std::ostream &os, LogEvent::ptr event) = 0;
    };

    void init();
private:
    /// 日志格式模板  输入
    std::string m_pattern;
    /// 日志格式解析后格式
    std::vector<FormatItem::ptr> m_items;
    /// 是否有错误
    bool m_error = false;

};


/**
 * @brief 日志输出目标
*/
class LogAppender {
public:
    typedef std::shared_ptr<LogAppender> ptr;

    LogAppender(LogFormatter::ptr default_formatter);
    virtual ~LogAppender() {}

    virtual void log(LogEvent::ptr event) = 0;

    void setFormatter(LogFormatter::ptr val);
    LogFormatter::ptr getFormatter() const;

    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level val) { m_level = val; }

    // virtual void log(LogEvent::ptr evnt) = 0;

    // virtual std::string toYamlString() = 0;

protected:
    LogLevel::Level m_level = LogLevel::DEBUG;
    LogFormatter::ptr m_formatter;
    LogFormatter::ptr m_defaultFormatter;
};

// 日志器
class Logger{
public:
    typedef std::shared_ptr<Logger> ptr;

    Logger(const std::string& name = "default");
    void log(LogEvent::ptr event);


    /* 设置日志输出地 */
    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    void clearAppenders();

    /* 增加删除日志级别 */
    LogLevel::Level getLevel() const { return m_level; };
    void setLevel(LogLevel::Level val) { m_level = val; };

    const std::string& getName() const { return m_name; }

private:
    std::string m_name;                         // 日志名称
    LogLevel::Level m_level;                    // 日志级别
    uint64_t m_createTime;                      // 创建时间
    std::list<LogAppender::ptr> m_appenders;    // appender集合
    LogFormatter::ptr m_formatter;              // 日志格式器
};

class LoggerManager {
public:
    LoggerManager();

    void init();

    Logger::ptr getLogger(const std::string& name);

    Logger::ptr getRoot() { return m_root; }
private:

    std::map<std::string, Logger::ptr> m_loggers;
    Logger::ptr m_root;
};

typedef webserver::Singleton<LoggerManager> LoggerMgr;


// 日志事件包装器，方便宏定义，内部包含日志事件和日志器 析构写日志
class LogEventWrap {
public:
    LogEventWrap(Logger::ptr Logger, LogEvent::ptr event);

    ~LogEventWrap();

    LogEvent::ptr getLogEvent() const { return m_event; }
private:
    Logger::ptr m_logger;
    LogEvent::ptr m_event;
};

// 输出到控制台的Appender
class StdoutLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;

    StdoutLogAppender();

    void log(LogEvent::ptr event) override;
};

// 输出到文件的Appender
class FileLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& filename);
    void log(LogEvent::ptr event) override;
    
    // 检查文件是否打开，已经打开返回true
    bool reopen();

private:
    std::string m_filename;
    std::ofstream m_filestream;
    uint64_t m_lastTime = 0;
    bool m_reopenError = false;
};

}   // end namespace webserver
#endif

