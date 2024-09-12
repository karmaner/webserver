#include <map>
#include <iostream>
#include <functional>
#include <time.h>
#include <string.h>

#include "log.h"
#include "config.h"
#include "util.h"
#include "macro.h"
#include "env.h"

namespace webserver {

const char *LogLevel::ToString(LogLevel::Level level) {
    switch (level) {
#define XX(name)                                                                                                       \
    case LogLevel::name:                                                                                               \
        return #name;                                                                                                  \
        break;

        XX(DEBUG)
        XX(INFO)
        XX(WARN)
        XX(ERROR)
        XX(FATAL)
#undef XX
    default:
        return "UNKNOW";
    }
    return "UNKNOW";
}

LogLevel::Level LogLevel::FromString(const std::string& str) {
#define XX(level, v)                                                                                                   \
    if (str == #v) {                                                                                                   \
        return LogLevel::level;                                                                                        \
    }
    XX(DEBUG, debug);
    XX(INFO, info);
    XX(WARN, warn);
    XX(ERROR, error);
    XX(FATAL, fatal);

    XX(DEBUG, DEBUG);
    XX(INFO, INFO);
    XX(WARN, WARN);
    XX(ERROR, ERROR);
    XX(FATAL, FATAL);
    return LogLevel::UNKNOW;
#undef XX
}

LogEventWrap::LogEventWrap(LogEvent::ptr event)
    :m_event(event) {
}

LogEventWrap::~LogEventWrap() {
    m_event->getLogger()->log(m_event->getLevel(), m_event);
}

std::stringstream& LogEventWrap::getSS() {
    return m_event->getSS();
}

void LogEvent::format(const char* fmt, ...) {
    va_list al;
    va_start(al, fmt);
    format(fmt, al);
    va_end(al);
}

void LogEvent::format(const char* fmt, va_list al) {
    char* buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if(len != -1) {
        m_ss << std::string(buf, len);
        free(buf);
    }
}

class MessageFormatItem : public LogFormatter::FormatItem {
public:
    MessageFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getContent();
    }
};

class LevelFormatItem : public LogFormatter::FormatItem {
public:
    LevelFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << LogLevel::ToString(level);
    }
};

class ElapseFormatItem : public LogFormatter::FormatItem {
public:
    ElapseFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getElapse();
    }
};

class NameFormatItem : public LogFormatter::FormatItem {
public:
    NameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getLogger()->getName();
    }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
    ThreadIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getThreadId();
    }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
    FiberIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFiberId();
    }
};

class ThreadNameFormatItem : public LogFormatter::FormatItem {
public:
    ThreadNameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getThreadName();
    }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
    DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S")
    :m_format(format) {
        if (m_format.empty()) {
            m_format = "%Y-%m-%d %H:%M:%S";
        }
    }

    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
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
    FilenameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFile();
    }
};

class LineFormatItem : public LogFormatter::FormatItem {
public:
    LineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getLine();
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    NewLineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << std::endl;
    }
};

class StringFormatItem : public LogFormatter::FormatItem {
public:
    StringFormatItem(const std::string& str)
        :m_string(str) {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << m_string;
    }
private:
    std::string m_string;
};

class TabFormatItem : public LogFormatter::FormatItem
{
public:
    TabFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << "\t";
    }
};

LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level
            ,const char* file, int32_t line, uint32_t elapse
            ,uint32_t thread_id, uint32_t fiber_id, uint64_t time
            ,const std::string& thread_name)
    :m_file(file)
    ,m_line(line)
    ,m_elapse(elapse)
    ,m_threadId(thread_id)
    ,m_fiberId(fiber_id)
    ,m_time(time)
    ,m_threadName(thread_name)
    ,m_logger(logger)
    ,m_level(level) {

}

class PercentSignFormatItem : public LogFormatter::FormatItem {
public:
    PercentSignFormatItem(const std::string &str) {
    }
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << "%";
    }
};

LogFormatter::LogFormatter(const std::string &pattern)
    :m_pattern(pattern) {
    init();
}

// 2016-02-0315:42:20[main:O]-[ DEBUG] This is debug message
void LogFormatter::init()
{
    // 按顺序存储解析到的pattern项
    // 每个pattern包括一个整数类型和一个字符串，类型为0表示该pattern是常规字符串，为1表示该pattern需要转义
    // 日期格式单独用下面的dataformat存储
    std::vector<std::pair<int, std::string>> patterns;
    std::string tmp;
    std::string dateformat;

    bool error = false;

    bool parsing_string = true;

    size_t i = 0;
    while (i < m_pattern.size()) {
        std::string c = std::string(1, m_pattern[i]);
        if (c == "%") {
            if (parsing_string) {
                if (!tmp.empty()) {
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
            if (parsing_string) {
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
                if (i < m_pattern.size() && m_pattern[i] != '{') {
                    continue;
                }
                i++;
                while (i < m_pattern.size() && m_pattern[i] != '}') {
                    dateformat.push_back(m_pattern[i]);
                    i++;
                }
                if (m_pattern[i] != '}') {
                    std::cout << "[ERROR] LogFormatter::init() " << "pattern: [" << m_pattern << "] '{' not closed"
                                << std::endl;
                    error = true;
                    break;
                }
                i++;
                continue;
            }
        }
    } // end whiel(i < m_pattern.size())

    if (error) {
        m_error = true;
        return;
    }

    if (!tmp.empty()) {
        patterns.push_back(std::make_pair(0, tmp));
        tmp.clear();
    }

    // // for debug
    // std::cout << "pattrens: " << std::endl;
    // for(auto &v : patterns) {
    //     std::cout << "type = " << v.first << ", value = " << v.second <<
    //     std::endl;
    // }
    // std::cout << "dataformat = " << dateformat << std::endl;

    static std::map<std::string, std::function<FormatItem::ptr(const std::string &str)>> s_format_items = {
#define XX(str, C)                                                                                                     \
    {                                                                                                                  \
        #str, [](const std::string& fmt) { return FormatItem::ptr(new C(fmt)); }                                       \
    }

        XX(m, MessageFormatItem),    // m:消息
        XX(p, LevelFormatItem),      // p:日志级别
        XX(r, ElapseFormatItem),     // r:累计毫秒数
        XX(c, NameFormatItem),       // c:日志名称
        XX(t, ThreadIdFormatItem),   // t:线程id
        XX(n, NewLineFormatItem),    // n:换行
        XX(d, DateTimeFormatItem),   // d:时间
        XX(f, FilenameFormatItem),   // f:文件名
        XX(l, LineFormatItem),       // l:行号
        XX(T, TabFormatItem),        // T:Tab
        XX(F, FiberIdFormatItem),    // F:协程id
        XX(N, ThreadNameFormatItem), // N:线程名称
#undef XX
    };

    for (auto& v : patterns) {
        if (v.first == 0) {
            m_items.push_back(FormatItem::ptr(new StringFormatItem(v.second)));
        } else if (v.second == "d") {
            m_items.push_back(FormatItem::ptr(new DateTimeFormatItem(dateformat)));
        } else {
            auto it = s_format_items.find(v.second);
            if (it == s_format_items.end()) {
                std::cout << "[ERROR] LogFormatter::init() " << "pattern: [" << m_pattern << "] "
                            << "unknown format item: " << v.second << std::endl;
                error = true;
                break;
            } else {
                m_items.push_back(it->second(v.second));
            }
        }
    }

    if (error) {
        m_error = true;
        return;
    }
}

std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    std::stringstream ss;
    for(auto& i : m_items) {
        i->format(ss, logger, level, event);
    }
    return ss.str();
}

std::ostream& LogFormatter::format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    for (auto& i : m_items) {
        i->format(ofs, logger, level, event);
    }
    return ofs;
}

void LogAppender::setFormatter(LogFormatter::ptr val) {
    MutexType::Lock lock(m_mutex);
    m_formatter = val;
    if (m_formatter) {
        m_hasFormatter = true;
    } else {
        m_hasFormatter = false;
    }
}

LogFormatter::ptr LogAppender::getFormatter() {
    MutexType::Lock lock(m_mutex);
    return m_formatter;
}

void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    if(level >= m_level) {
        MutexType::Lock lock(m_mutex);
        m_formatter->format(std::cout, logger, level, event);
    }
}

std::string StdoutLogAppender::toYamlString() {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["type"] = "StdoutLogAppender";
    if(m_level != LogLevel::UNKNOW) {
        node["level"] = LogLevel::ToString(m_level);
    }
    if(m_hasFormatter && m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

FileLogAppender::FileLogAppender(const std::string& filename)
    :m_filename(filename) {
    reopen();
}

void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    if(level >= m_level) {
        uint64_t now = event->getTime();
        if(now >= (m_lastTime + 3)) {
            reopen();
            m_lastTime = now;
        }
        MutexType::Lock lock(m_mutex);
        if(!m_formatter->format(m_filestream, logger, level, event)) {
            std::cout << "error" << std::endl;
        }
    }
}

bool FileLogAppender::reopen() {
    MutexType::Lock lock(m_mutex);
    if (m_filestream) {
        m_filestream.close();
    }

    return FSUtil::OpenForWrite(m_filestream, m_filename, std::ios::app);
}

std::string FileLogAppender::toYamlString() {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["type"] = "FileLogAppender";
    node["file"] = m_filename;
    if (m_level != LogLevel::UNKNOW) {
        node["level"] = LogLevel::ToString(m_level);
    }
    if (m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

Logger::Logger(const std::string& name)
    :m_name(name)
    ,m_level(LogLevel::DEBUG) {
    m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
}

void Logger::addAppender(LogAppender::ptr appender) {
    MutexType::Lock lock(m_mutex);
    if (!appender->getFormatter()) {
        MutexType::Lock ll(appender->m_mutex);
        appender->m_formatter = m_formatter;
    }
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
    MutexType::Lock lock(m_mutex);
    for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it) {
        if (*it == appender) {
            m_appenders.erase(it);
            break;
        }
    }
}

void Logger::clearAppenders() {
    MutexType::Lock lock(m_mutex);
    m_appenders.clear();
}

void Logger::setFormatter(LogFormatter::ptr val)
{
    MutexType::Lock lock(m_mutex);
    m_formatter = val;

    for(auto& i : m_appenders) {
        MutexType::Lock ll(i->m_mutex);
        if(!i->m_hasFormatter) {
            i->m_formatter = m_formatter;
        }
    }
}

void Logger::setFormatter(const std::string& val) {
    std::cout << "---" << val << std::endl;
    webserver::LogFormatter::ptr new_val(new webserver::LogFormatter(val));
    if(new_val->isError()) {
        std::cout << "Logger setFormatter name=" << m_name
                    << " value=" << val << " invalid formatter"
                    << std::endl;
        return;
    }
    //m_formatter = new_val;
    setFormatter(new_val);;
}

LogFormatter::ptr Logger::getFormatter() {
    MutexType::Lock lock(m_mutex);
    return m_formatter;
}

std::string Logger::toYamlString() {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["name"] = m_name;

    if (m_level != LogLevel::UNKNOW) {
        node["level"] = LogLevel::ToString(m_level);
    }

    if (m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }

    for (auto& i : m_appenders) {
        node["appenders"].push_back(YAML::Load(i->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
    if(level >= m_level) {
        auto self = shared_from_this();
        MutexType::Lock lock(m_mutex);
        if(!m_appenders.empty()) {
            for(auto& i : m_appenders) {
                i->log(self, level, event);
            }
        } else if(m_root) {
            m_root->log(level, event);
        }
    }
}

void Logger::debug(LogEvent::ptr event) {
    log(LogLevel::DEBUG, event);
}

void Logger::info(LogEvent::ptr event) {
    log(LogLevel::INFO, event);
}

void Logger::warn(LogEvent::ptr event) {
    log(LogLevel::WARN, event);
}

void Logger::error(LogEvent::ptr event) {
    log(LogLevel::ERROR, event);
}

void Logger::fatal(LogEvent::ptr event) {
    log(LogLevel::FATAL, event);
}

LoggerManager::LoggerManager()
{
    m_root.reset(new Logger);
    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));

    m_loggers[m_root->m_name] = m_root;

    init();
}

Logger::ptr LoggerManager::getLogger(const std::string &name) {
    MutexType::Lock lock(m_mutex);
    auto it = m_loggers.find(name);
    if(it != m_loggers.end()) {
        return it->second;
    }

    Logger::ptr logger(new Logger(name));
    logger->m_root = m_root;
    m_loggers[name] = logger;
    return logger;
}

// TODO: 从配置文件加载
void LoggerManager::init()
{
}



struct LogAppenderDefine {
    int type = 0;
    LogLevel::Level level = LogLevel::UNKNOW;
    std::string formatter;
    std::string file;

    bool operator==(const LogAppenderDefine &oth) const {
        return type == oth.type 
            && level == oth.level 
            && formatter == oth.formatter
            && file == oth.file;
    }
};

struct LogDefine {
    std::string name;
    LogLevel::Level level = LogLevel::UNKNOW;
    std::string formatter;
    std::vector<LogAppenderDefine> appenders;

    bool operator==(const LogDefine &oth) const {
        return name == oth.name && level == oth.level && formatter == oth.formatter && appenders == appenders;
    }

    bool operator<(const LogDefine &oth) const { // 为什么加了const就不报错了
        return name < oth.name;
    }
};

// string类型 到 logDefine类型
template <> class LexicalCast<std::string, std::set<LogDefine>> {
public:
    std::set<LogDefine> operator()(const std::string &v) {
        YAML::Node node = YAML::Load(v);
        std::set<LogDefine> vec;
        for (size_t i = 0; i < node.size(); ++i)
        {
            auto n = node[i];
            if (!n["name"].IsDefined()) {
                std::cout << "log config error: name is null, " << n << std::endl;
                continue;
            }
            LogDefine ld;
            ld.name = n["name"].as<std::string>();
            ld.level = LogLevel::FromString((n["level"]).IsDefined() ? n["level"].as<std::string>() : "");
            if (n["formatter"].IsDefined()) {
                ld.formatter = n["formatter"].as<std::string>();
            }

            if ((n["appenders"]).IsDefined()) {
                std::cout << "==" << ld.name << " = " << n["appenders"].size() << std::endl;
                for (size_t x = 0; x < n["appenders"].size(); ++x) {
                    auto a = n["appenders"][x];
                    if (!a["type"].IsDefined()) {
                        std::cout << "log config error: appender type is null, " << a << std::endl;
                        continue;
                    }
                    std::string type = a["type"].as<std::string>();
                    LogAppenderDefine lad;
                    if (type == "FileLogAppender") {
                        lad.type = 1;
                        if (!a["file"].IsDefined()) {
                            std::cout << "log config error: fileappender file is null, " << a << std::endl;
                            continue;
                        }
                        lad.file = a["file"].as<std::string>();
                        if (a["formatter"].IsDefined()) {
                            lad.formatter = a["formatter"].as<std::string>();
                        }
                    } else if (type == "StdoutLogAppender") {
                        lad.type = 2;
                    }
                    else {
                        std::cout << "log config error: appender type is invalid, " << a << std::endl;
                        continue;
                    }

                    ld.appenders.push_back(lad);
                }
            }
            std::cout << "---" << ld.name << " - " << ld.appenders.size() << std::endl;
            vec.insert(ld);
        } // end of for
        return vec;
    }

}; // end of class LexicalCast

// LogDefine --> YAML string
template <>
class LexicalCast<std::set<LogDefine>, std::string> {
public:
    std::string operator()(const std::set<LogDefine> &v) {
        YAML::Node node;
        for (auto &i : v) {
            YAML::Node n;
            n["name"] = i.name;
            if (i.level != LogLevel::UNKNOW) {
                n["level"] = LogLevel::ToString(i.level);
            }
            if (i.formatter.empty()) {
                n["formatter"] = i.formatter;
            }

            for (auto &a : i.appenders) {
                YAML::Node na;
                if (a.type == 1) {
                    na["type"] = "FileAppender";
                    na["file"] = a.file;
                } else if (a.type == 2) {
                    na["type"] = "StdoutLogAppender";
                }

                if (a.level != LogLevel::UNKNOW) {
                    na["level"] = LogLevel::ToString(a.level);
                }

                if (!a.formatter.empty()) {
                    na["formatter"] = a.formatter;
                }

                n["appender"].push_back(na);
            }
            node.push_back(n);
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

webserver::ConfigVar<std::set<LogDefine>>::ptr g_log_defines =
    webserver::Config::Lookup("logs", std::set<LogDefine>(), "logs config");

struct LogIniter {
    LogIniter() {
        g_log_defines->addListener([](const std::set<LogDefine> &old_value, 
                    const std::set<LogDefine> &new_value) {
            WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << "on_logger_conf_changed";
            for (auto &i : new_value) {
                auto it = old_value.find(i);
                webserver::Logger::ptr logger;
                if (it == old_value.end()) {
                    logger = WEBSERVER_LOG_NAME(i.name);
                } else {
                    if (!(i == *it)) {
                        logger = WEBSERVER_LOG_NAME(i.name);
                    }
                }
                logger->setLevel(i.level);
                
                if (!i.formatter.empty()) {
                    logger->setFormatter(i.formatter);
                }

                logger->clearAppenders();
                for (auto &a : i.appenders) {
                    webserver::LogAppender::ptr ap;
                    if (a.type == 1) {
                        ap.reset(new FileLogAppender(a.file));
                    }
                    else if (a.type == 2) {
                        if(!webserver::EnvMgr::GetInstance()->has("d")) {
                        ap.reset(new StdoutLogAppender());
                        } else {
                            continue;
                        }
                    }
                    ap->setLevel(a.level);
                    logger->addAppender(ap);
                }
            }

            for (auto& i : old_value) {
                auto it = new_value.find(i);
                if (it == new_value.end()) {
                    auto logger = WEBSERVER_LOG_NAME(i.name);
                    logger->setLevel((LogLevel::Level)100);
                    logger->clearAppenders();
                }
            }
        });
    }
};

static LogIniter __log_init;

std::string LoggerManager::ToYamlString() {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    for (auto &i : m_loggers) {
        node.push_back(YAML::Load(i.second->toYamlString()));
    }
    std::stringstream ss;
    ss << std::endl;
    return ss.str();
}

} // namespace webserver
