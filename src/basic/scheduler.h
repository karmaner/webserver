#ifndef __SRC_SCHEDULER_H__
#define __SRC_SCHEDULER_H__

#include <memory>
#include <vector>
#include <list>
#include <iostream>

#include "fiber.h"
#include "thread.h"

namespace webserver {

/**
 *   封装的是N-M的协程调度器
 *     内部有一个线程池,支持协程在线程池里面切换
 */
class Scheduler {
public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef Mutex MutexType;

    Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = ""); 
    virtual ~Scheduler();

    const std::string& getName() const { return m_name; }

    static Scheduler* GetThis();        // 返回当前协程调度器
    static Fiber* GetMainFiber();       // 返回当前协程调度器的调度协程

    void start();
    void stop();

    // 调度协程
    template<class FiberOrCb>
    void schedule(FiberOrCb fc, int thread = -1) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            need_tickle = scheduleNoLock(fc, thread);
        }

        if(need_tickle) {
            tickle();
        }
    }

    // 批量调度协程
    template<class InputIterator>
    void schedule(InputIterator begin, InputIterator end)
    {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            while(begin != end) {
                need_tickle = scheduleNoLock(&*begin, -1) || need_tickle;
                ++begin;
            }
        }

        if(need_tickle)
        {
            tickle();
        }
    }

    void switchTo(int thread = -1);
    std::ostream& dump(std::ostream& os);

protected:
    virtual void tickle();  // 通知协程调度器 任务提醒
    void run();             // 协程调度函数
    virtual bool stopping();// 返回是否可以停止
    virtual void idle();    // 闲杂协程

    void setThis();         // 设置当前的协程调度器
    bool hasIdleThreads() { return m_idleThreadCount > 0; } // 是否有空闲线程

private:
    // 加入任务队列 协程调度启动(无锁)
    template<class FiberOrCb> 
    bool scheduleNoLock(FiberOrCb fc, int thread) {
        bool need_tickle = m_fibers.empty();
        FiberAndThread ft(fc, thread);
        if(ft.fiber || ft.cb) {
            m_fibers.push_back(ft);
        }
        return need_tickle;
    }

private:
    /**
     * 封装任务 这个任务可以是函数、协程  
     */
    struct FiberAndThread {
        Fiber::ptr fiber;
        std::function<void()> cb;
        int thread; // 线程ID

        FiberAndThread(Fiber::ptr f, int thr) 
            :fiber(f)
            ,thread(thr) {
        }

        FiberAndThread(Fiber::ptr* f, int thr) 
        :thread(thr) {
            fiber.swap(*f);
        }

        FiberAndThread(std::function<void()> f, int thr)
            :cb(f)
            ,thread(thr) { }

        FiberAndThread(std::function<void()>* f, int thr)
            :thread(thr) {
            cb.swap(*f);        
        }

        FiberAndThread()
            :thread(-1) { }

        void reset() {
            fiber = nullptr;
            cb = nullptr;
            thread = -1;
        }
    };

private:
    MutexType m_mutex;
    std::vector<Thread::ptr> m_threads;  // 线程池
    std::list<FiberAndThread> m_fibers;  // 任务队列  以协程为单位
    Fiber::ptr m_rootFiber;              // use_caller为true时有效，调度器所在线程的调用协程(但它是子协程)
    std::string m_name;                  // 协程调度器名称

protected:

    std::vector<int> m_threadIds;        // 所有协程id
    size_t m_threadCount = 0;            // 线程数
    std::atomic<size_t> m_activeThreadCount = {0};
    std::atomic<size_t> m_idleThreadCount = {0};

    bool m_stopping = true;
    bool m_autoStop = false;
    int m_rootThread = 0;
    
};

class SchedulerSwitcher : public Noncopyable {
public:
    SchedulerSwitcher(Scheduler* target = nullptr);
    ~SchedulerSwitcher();
private:
    Scheduler* m_caller;
};

} // end of namespace


#endif