#ifndef __SRC_IOMANAGER_H__
#define __SRC_IOMANAGER_H__

#include "scheduler.h"

namespace webserver {

class IOManager : public Scheduler {
public:
    typedef std::shared_ptr<IOManager> ptr;
    typedef RWMutex RWMutexType;

    enum Event {
        NONE = 0x0, // 无
        READ = 0x1, // 读
        WRITE = 0x4, // 写
    };

private:
    // socket fd上下文类
    struct FdContext {
        typedef Mutex MutexType;
        
        // 事件上下文类
        struct EventContext {
            Scheduler* scheduler = nullptr; // 执行事件回调的调度器
            Fiber::ptr fiber;               // 事件回调协程
            std::function<void()> cb;       // call back
        };

        EventContext& getEventContext(Event event);
        void resetContext(EventContext& ctx);
        void triggerEvent(Event event);

        EventContext read;
        EventContext write;

        int fd = 0; // 事件关联句柄
        Event events = NONE;
        MutexType mutex;
    }
public:
    IOManager(size_t threads = 1, bool use_call = true, const::string& name = "");
    ~IOManager();

    int addEvent(int fd, Event event, std::function<void()> cb = nullptr);
    bool delEvent(int fd, Event event);
    bool cancelEvent(int fd, Event event);

    bool cancelAll(int fd);

    static IOManager* GetThis();

private:
    RWMutexType m_mutex;

    int m_epfd = 0;     // epoll 文件句柄
    int m_tickleFds[2]; // pipe 文件句柄， fd[0]读端, f[1]写端
    std::atomic<size_t> m_pendingEventCount = {0};  // 当前等待执行的IO事件数量
    std::vector<FdContext*> m_fdContexts;    // socket事件上下文的容器
}



}

#endif