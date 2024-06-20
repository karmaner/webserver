#include "log.h"
#include <iostream>
#include <map>
#include <functional>

namespace webserver {

const char* LogLevel::ToString(LogLevel::Level level) {
    switch(level) {
#define XX(name) \
    case LogLevel::name: \
        return #name; \
        break;
    
    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
    XX(FATAL);
#undef XX
    default:
        return "UNKNOW";
    }
    return "UNKNOW";
}

LogEvent::LogEvent(const std::string &logger_name, LogLevel::Level level, const char *file, int32_t line
        , int64_t elapse, uint32_t thread_id, uint64_t fiber_id, time_t time
        , const std::string &thread_name)
    : m_loggerName(logger_name)
    , m_level(level)
    , m_file(file)
    , m_line(line)
    , m_elapse(elapse)
    , m_threadId(thread_id)
    , m_fiberId(fiber_id)
    , m_time(time)
    , m_threadName(thread_name) {
}

void LogEvent::printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

void LogEvent::vprintf(const char* fmt, va_list ap) {
    char* buf = nullptr;
    int len = vasprintf(&buf, fmt, ap);
    if(len != 1) {
        m_ss << std::string(buf, len);
        free(buf);
    }
}


class MessageFormatItem : public LogFormatter::FormatItem {
public:
    MessageFormatItem(const std::string& str) {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getContent();
    }
};

class LevelFormatItem : public LogFormatter::FormatItem {
public:
    LevelFormatItem(const std::string& str) {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getLevel();
    }
};

class ElapseFormatItem : public LogFormatter::FormatItem {
public:
    ElapseFormatItem(const std::string& str) {}
    void format(std::ostream& os,LogEvent::ptr event) override {
        os << event->getElapse();
    }
};

class NameFormatItem : public LogFormatter::FormatItem {
public:
    NameFormatItem(const std::string& str) {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event-> getLoggerName();
    }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
    ThreadIdFormatItem(const std::string& str) {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getThreadID();
    }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
    FiberIdFormatItem(const std::string& str) {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getFiberId();
    }
};

class ThreadNameFormatItem : public LogFormatter::FormatItem {
public:
    ThreadNameFormatItem(const std::string& str) {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getThreadName();
    }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
    DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S")
        :m_format(format) {
        if(m_format.empty()) {
            m_format = "%Y-%m-%d %H:%M:%S";
        }
    }

    void format(std::ostream& os, LogEvent::ptr event) override {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }
private:
    std::string m_format;
};

class FilenameFormatItem : public LogFormatter::FormatItem {
public:
    FilenameFormatItem(const std::string& str) {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getFile();
    }
};

class LineFormatItem : public LogFormatter::FormatItem {
public:
    LineFormatItem(const std::string& str) {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getLine();
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    NewLineFormatItem(const std::string& str) {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << std::endl;
    }
};

class StringFormatItem : public LogFormatter::FormatItem {
public:
    StringFormatItem(const std::string& str)
        :FormatItem(str), m_string(str) { }
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << m_string;
    }
private:
    std::string m_string;
};

class TabFormatItem : public LogFormatter::FormatItem {
public:
    TabFormatItem(const std::string &str) {}
    void format(std::ostream &os, LogEvent::ptr event) override {
        os << "\t";
    }
};

class PercentSignFormatItem : public LogFormatter::FormatItem {
public:
    PercentSignFormatItem(const std::string &str) {}
    void format(std::ostream &os, LogEvent::ptr event) override {
        os << "%";
    }
};

LogFormatter::LogFormatter(const std::string &pattern)
    : m_pattern(pattern) {
    init();
}

// 2016-02-0315:42:20[main:O]-[ DEBUG] This is debug message
void LogFormatter::init() {
    // 按顺序存储解析到的pattern项
    // 每个pattern包括一个整数类型和一个字符串，类型为0表示该pattern是常规字符串，为1表示该pattern需要转义
    // 日期格式单独用下面的dataformat存储
    std::vector<std::pair<int, std::string>> patterns;
    std::string tmp;
    std::string dateformat;

    bool error = false;

    bool parsing_string = true;

    size_t i = 0;
    while(i < m_pattern.size()) {
        std::string c = std::string(1, m_pattern[i]);
        if(c == "%") {
            if(parsing_string) {
                if(!tmp.empty()) {
                    patterns.push_back(std::make_pair(0, tmp));
                }
                tmp.clear();
                parsing_string = false;
                i++;
                continue;
            } else {
                patterns.push_back(std::make_pair(1, c));
                parsing_string = true;
                i++;
                continue;
            }
        } else { // not %
            if(parsing_string) {
                tmp += c;
                i++;
                continue;
            } else {
                patterns.push_back(std::make_pair(1, c));
                parsing_string = true;

                if (c != "d") {
                    i++;
                    continue;
                }
                i++;
                if(i < m_pattern.size() && m_pattern[i] != '{') {
                    continue;
                }
                i++;
                while( i < m_pattern.size() && m_pattern[i] != '}') {
                    dateformat.push_back(m_pattern[i]);
                    i++;
                }
                if(m_pattern[i] != '}') {
                    std::cout << "[ERROR] LogFormatter::init() " << "pattern: [" << m_pattern << "] '{' not closed" << std::endl;
                    error = true;
                    break;
                }
                i++;
                continue;
            }
        }
    } // end whiel(i < m_pattern.size())

    if(error) {
        m_error = true;
        return;
    }

    if(!tmp.empty()) {
        patterns.push_back(std::make_pair(0, tmp));
        tmp.clear();
    }

    // // for debug
    // std::cout << "pattrens: " << std::endl;
    // for(auto &v : patterns) {
    //     std::cout << "type = " << v.first << ", value = " << v.second << std::endl;
    // }
    // std::cout << "dataformat = " << dateformat << std::endl;

    static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)> > s_format_items = {
#define XX(str, C) \
        {#str, [](const std::string& fmt) { return FormatItem::ptr(new C(fmt));}}

        XX(m, MessageFormatItem),           //m:消息
        XX(p, LevelFormatItem),             //p:日志级别
        XX(r, ElapseFormatItem),            //r:累计毫秒数
        XX(c, NameFormatItem),              //c:日志名称
        XX(t, ThreadIdFormatItem),          //t:线程id
        XX(n, NewLineFormatItem),           //n:换行
        XX(d, DateTimeFormatItem),          //d:时间
        XX(f, FilenameFormatItem),          //f:文件名
        XX(l, LineFormatItem),              //l:行号
        XX(T, TabFormatItem),               //T:Tab
        XX(F, FiberIdFormatItem),           //F:协程id
        XX(N, ThreadNameFormatItem),        //N:线程名称
#undef XX
    };

    for(auto& v : patterns) {
        if(v.first == 0) {
            m_items.push_back(FormatItem::ptr(new StringFormatItem(v.second)));
        } else if(v.second == "d") {
            m_items.push_back(FormatItem::ptr(new DateTimeFormatItem(dateformat)));
        } else {
            auto it = s_format_items.find(v.second);
            if(it == s_format_items.end()) {
                std::cout << "[ERROR] LogFormatter::init() " << "pattern: [" << m_pattern << "] " << "unknown format item: " << v.second << std::endl;
                error = true;
                break;
            } else {
                m_items.push_back(it->second(v.second));
            }
        }
    }

    if(error) {
        m_error = true;
        return;
    }

}

std::string LogFormatter::format(LogEvent::ptr event) {
    std::stringstream ss;
    for(auto& i : m_items) {
        i->format(ss, event);
    }
    return ss.str();
}

std::ostream& LogFormatter::format(std::ostream& os, LogEvent::ptr event) {
    for(auto& i : m_items) {
        i->format(os, event);
    }
    return os;
}

LogAppender::LogAppender(LogFormatter::ptr default_formatter)
    : m_defaultFormatter(default_formatter) {
}

void LogAppender::setFormatter(LogFormatter::ptr val) {
    m_formatter = val;
}

LogFormatter::ptr LogAppender::getFormatter() const{
    return m_formatter ? m_formatter : m_defaultFormatter;
}

StdoutLogAppender::StdoutLogAppender()
    : LogAppender(LogFormatter::ptr(new LogFormatter)) {
}

void StdoutLogAppender::log(LogEvent::ptr event)  {
    if(m_formatter) {
        m_formatter->format(std::cout, event);
    } else {
        m_defaultFormatter->format(std::cout, event);
    }
}


FileLogAppender::FileLogAppender(const std::string& filename)
    : LogAppender(LogFormatter::ptr(new LogFormatter)) {
    m_filename = filename;
    reopen();
    if(m_reopenError) {
        std::cout << "reopen file " << m_filename << " error"  << std::endl;
    }
}

void FileLogAppender::log(LogEvent::ptr event)  {
    uint64_t now = event->getTime();
    if(now >= (m_lastTime + 3)) {
        reopen();
        if(m_reopenError) {
            std::cout << "reopen file " << m_filename << " error" << std::endl;
        }
        m_lastTime = now;
    }
    if(m_reopenError) return;

    if(m_formatter) {
        if(!m_formatter->format(m_filestream, event)) {
            std::cout << "[ERROR] FileLogAppender::log(） format error" << std::endl;
        }
    } else {
        if(!m_defaultFormatter->format(m_filestream, event)) {
            std::cout << "[ERROR] FileLogAppender::log() format error" << std::endl;
        }
    }
}

bool FileLogAppender::reopen() {
    if(m_filestream) {
        m_filestream.close();
    }
    m_filestream.open(m_filename, std::ios::app);
    m_reopenError = !m_filestream;
    return !m_reopenError;
}



Logger::Logger(const std::string &name)
    : m_name(name)
    , m_level(LogLevel::DEBUG) {

        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);

        // 将 timespec 结构体转换为 uint64_t 类型
        m_createTime = (uint64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;

}


void Logger::addAppender(LogAppender::ptr appender) {
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
    for(auto it = m_appenders.begin();
                it != m_appenders.end(); ++it ){
                    if (*it == appender) {
                        m_appenders.erase(it);
                        break;
                    }
                }
} 

void Logger::clearAppenders() {
    m_appenders.clear();
}

void Logger::log(LogEvent::ptr event) {
    if(event->getLevel() >= m_level) {
        for(auto& i : m_appenders) {
            i->log(event);
        }
    }
}

LoggerManager::LoggerManager() {
    m_root.reset(new Logger("root"));
    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));
    m_loggers[m_root->getName()] = m_root;
    init();
}

Logger::ptr LoggerManager::getLogger(const std::string &name) {
    auto it = m_loggers.find(name);
    if(it != m_loggers.end()) {
        return it->second;
    }

    // 没有就会new一个Logger
    Logger::ptr logger(new Logger(name));
    m_loggers[name] = logger;
    return logger;
}

// TODO: 从配置文件加载
void LoggerManager::init() {

}

LogEventWrap::LogEventWrap(Logger::ptr logger, LogEvent::ptr event) 
    : m_logger(logger)
    , m_event(event) {

}

LogEventWrap::~LogEventWrap() {
    m_logger->log(m_event);
}

void Logger::debug(LogEvent::ptr event) {
    log(event);
}

void Logger::info(LogEvent::ptr event) {
    log(event);

}
void Logger::warn(LogEvent::ptr event) {
    log(event);
}
void Logger::error(LogEvent::ptr event) {
    log(event);
}
void Logger::fatal(LogEvent::ptr event) {
    log(event);
}


};