#ifndef __SRC_LOG_H__
#define __SRC_LOG_H__

#include "singleton.h"
#include "util.h"
#include <chrono>
#include <fstream>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <stdarg.h>
#include <stdint.h>
#include <string>
#include <vector>
#include "thread.h"

#define WEBSERVER_LOG_ROOT() webserver::LoggerMgr::GetInstance()->getRoot()

#define WEBSERVER_LOG_NAME(name) webserver::LoggerMgr::GetInstance()->getLogger(name)

#define WEBSERVER_LOG_LEVEL(logger, level)                                                                              \
    if(logger->getLevel() <= level)                                                                                     \
        webserver::LogEventWrap(webserver::LogEvent::ptr(new webserver::LogEvent(logger, level,                         \
                        __FILE__, __LINE__, 0, webserver::GetThreadId(),                                                \
                webserver::GetFiberId(), time(0), webserver::Thread::GetName()))).getSS()

#define WEBSERVER_LOG_FATAL(logger) WEBSERVER_LOG_LEVEL(logger, webserver::LogLevel::FATAL)
#define WEBSERVER_LOG_ERROR(logger) WEBSERVER_LOG_LEVEL(logger, webserver::LogLevel::ERROR)
#define WEBSERVER_LOG_WARN(logger) WEBSERVER_LOG_LEVEL(logger, webserver::LogLevel::WARN)
#define WEBSERVER_LOG_INFO(logger) WEBSERVER_LOG_LEVEL(logger, webserver::LogLevel::INFO)
#define WEBSERVER_LOG_DEBUG(logger) WEBSERVER_LOG_LEVEL(logger, webserver::LogLevel::DEBUG)

#define WEBSERVER_LOG_FMT_LEVEL(logger, level, fmt, ...)                                                               \
    if (logger->getLevel() <= level)                                                                                   \
        webserver::LogEventWrap(webserver::LogEvent::ptr(new webserver::LogEvent(logger, level,                        \
                        __FILE__, __LINE__, 0, webserver::GetThreadId(),                                               \
                webserver::GetFiberId(), time(0), webserver::Thread::GetName()))).getEvent()->format(fmt, __VA_ARGS__)

#define WEBSERVER_LOG_FMT_FATAL(logger, fmt, ...)                                                                      \
    WEBSERVER_LOG_FMT_LEVEL(logger, webserver::LogLevel::FATAL, fmt, __VA_ARGS__)
#define WEBSERVER_LOG_FMT_ERROR(logger, fmt, ...)                                                                      \
    WEBSERVER_LOG_FMT_LEVEL(logger, webserver::LogLevel::ERROR, fmt, __VA_ARGS__)
#define WEBSERVER_LOG_FMT_WARN(logger, fmt, ...)                                                                       \
    WEBSERVER_LOG_FMT_LEVEL(logger, webserver::LogLevel::WARN, fmt, __VA_ARGS__)
#define WEBSERVER_LOG_FMT_INFO(logger, fmt, ...)                                                                       \
    WEBSERVER_LOG_FMT_LEVEL(logger, webserver::LogLevel::INFO, fmt, __VA_ARGS__)
#define WEBSERVER_LOG_FMT_DEBUG(logger, fmt, ...)                                                                      \
    WEBSERVER_LOG_FMT_LEVEL(logger, webserver::LogLevel::DEBUG, fmt, __VA_ARGS__)

namespace webserver
{

class Logger;
class LoggerManager;

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

    static const char *ToString(LogLevel::Level level);

    // YAML sting转化为sting
    static LogLevel::Level FromString(const std::string &str);
};


// 日志事件
class LogEvent {
public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level
            ,const char* file, int32_t line, uint32_t elapse
            ,uint32_t thread_id, uint32_t fiber_id, uint64_t time
            ,const std::string& thread_name);

    const char *getFile() const { return m_file; }                  // 文件名
    uint32_t getLine() const { return m_line; }                      // 行号
    LogLevel::Level getLevel() { return m_level; }                  // 获取日志级别
    uint64_t getElapse() const { return m_elapse; }                 // 程序启动到现在的毫秒
    uint32_t getThreadId() const { return m_threadId; }             // 线程id
    uint32_t getFiberId() const { return m_fiberId; }               // 协程id
    std::string getThreadName() { return m_threadName; }            // 线程名
    uint64_t getTime() const { return m_time; }                     // 时间戳
    const std::string getContent() const { return m_ss.str(); }     // 返回日志内容
    std::shared_ptr<Logger> getLogger() const { return m_logger; }  // 返回日志器
    std::stringstream &getSS() { return m_ss; }


    void format(const char *fmt, ...);
    void format(const char *fmt, va_list al);

private:
    const char *m_file = nullptr; // 文件名
    uint32_t m_line = 0;          // 行号
    uint64_t m_elapse = 0;        // 程序启动到现在的毫秒
    uint32_t m_threadId = 0;      // 线程id
    uint32_t m_fiberId = 0;       // 协程id
    uint64_t m_time = 0;          // 时间戳
    std::string m_threadName;     // 线程名
    std::stringstream m_ss;       // 日志内容流
    
    std::shared_ptr<Logger> m_logger;
    LogLevel::Level m_level;      // 日志级别

};

// 日志事件包装器，方便宏定义，内部包含日志事件和日志器 析构写日志
class LogEventWrap {
public:
    LogEventWrap(LogEvent::ptr event);

    ~LogEventWrap();

    LogEvent::ptr getEvent() const { return m_event; }

    std::stringstream& getSS();
private:
    LogEvent::ptr m_event;
};

// 日志格式器
class LogFormatter {
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(const std::string &pattern);

    // %t   %thread_id %m%n
    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
    std::ostream& format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);

public:
    class FormatItem {
    public:
        typedef std::shared_ptr<FormatItem> ptr;

        virtual ~FormatItem() { }
        virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    };



void init();
bool isError() const { return m_error; }
const std::string getPattern() const { return m_pattern; }

private:
    std::string m_pattern;                  // 日志格式模板  输入
    std::vector<FormatItem::ptr> m_items;   // 日志格式解析后格式
    bool m_error = false;                   // 解析是否有错误
};

/**
 * @brief 日志输出目标
 */
class LogAppender {
friend class Logger;
public:
    typedef std::shared_ptr<LogAppender> ptr;
    typedef Spinlock MutexType;

    virtual ~LogAppender() {}

    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    virtual std::string toYamlString() = 0;

    void setFormatter(LogFormatter::ptr val);
    LogFormatter::ptr getFormatter();

    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level val) { m_level = val; }

protected:
    LogLevel::Level m_level = LogLevel::DEBUG;
    LogFormatter::ptr m_formatter;
    bool m_hasFormatter = false;
    MutexType m_mutex;
};

// 日志器
class Logger : public std::enable_shared_from_this<Logger>{
friend class LoggerManager;
public:
    typedef std::shared_ptr<Logger> ptr;
    typedef Spinlock MutexType;

    Logger(const std::string& name = "root");
    void log(LogLevel::Level level, LogEvent::ptr event);
    
    /* 日志输出级别 */
    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    /* 设置日志输出地 */
    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    void clearAppenders();

    /* 增加删除日志级别 */
    LogLevel::Level getLevel() const { return m_level; };
    void setLevel(LogLevel::Level val) { m_level = val; };

    const std::string &getName() const { return m_name; }

    void setFormatter(LogFormatter::ptr val);
    void setFormatter(const std::string& val);
    LogFormatter::ptr getFormatter();

    std::string toYamlString();

private:
    std::string m_name;                      // 日志名称
    LogLevel::Level m_level;                 // 日志级别
    std::list<LogAppender::ptr> m_appenders; // appender集合
    LogFormatter::ptr m_formatter;           // 日志格式器
    Logger::ptr m_root;
    MutexType m_mutex;
};

// 输出到控制台的Appender
class StdoutLogAppender : public LogAppender
{
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;

    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
    std::string toYamlString() override;
};

// 输出到文件的Appender
class FileLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<FileLogAppender> ptr;

    FileLogAppender(const std::string& filename);
    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
    std::string toYamlString() override;

    // 检查文件是否打开，已经打开返回true
    bool reopen();

private:
    std::string m_filename;
    std::ofstream m_filestream;
    uint64_t m_lastTime = 0;
};

class LoggerManager {
public:
    typedef Spinlock MutexType;
    LoggerManager();

    Logger::ptr getLogger(const std::string &name);

    void init();
    Logger::ptr getRoot() { return m_root; } 

    std::string ToYamlString();
    
private:
    std::map<std::string, Logger::ptr> m_loggers;
    Logger::ptr m_root;
    MutexType m_mutex;
};

// 日志管理类 单例模式
typedef webserver::Singleton<LoggerManager> LoggerMgr;

} // end namespace webserver
#endif
