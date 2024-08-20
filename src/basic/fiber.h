#ifndef __SRC_FIBER_H__
#define __SRC_FIBER_H__

#include <memory>
#include <functional>
#include <ucontext.h>
#include "thread.h"

namespace webserver {

class Scheduler;

class Fiber : public std::enable_shared_from_this<Fiber> {
friend class Scheduler;
public:
    typedef std::shared_ptr<Fiber> ptr;

    enum State {
        
        INIT,   // 初始化状态
        HOLD,   // 暂停状态
        EXEC,   // 执行中状态
        TERM,   // 结束状态
        READY,  // 可执行状态
        EXCEPT  // 异常状态
    };

private:
    Fiber();
public: 
    Fiber(std::function<void()> cb, size_t stacksize = 0, bool use_caller = false);
    ~Fiber();

    // 重置协程函数， 并重置状态
    void reset(std::function<void()> cb);
    // 切换当前协程
    void swapIn();
    // 切换到后台执行
    void swapOut();

    // 线程切换到前台
    void call();
    // 线程切换到后台
    void back();

    uint64_t getId() const { return m_id; }

    void setState(State s) { m_state = s; }
    State getState() const { return m_state; }

public:
    // 设置当前线程的运行协程
    static void SetThis(Fiber* f);
    // 返回当前所在的协程
    static Fiber::ptr GetThis();
    
    // 将当前协程切换到后台,并设置为READY状态
    static void YieldToReady();
    // 将当前协程切换到后台,并设置为HOLD状态
    static void YieldToHold();

    static uint64_t TotalFibers();
    static uint64_t GetFiberId();
    
    static void MainFunc();
    static void callerMainFunc();

private:
    uint64_t m_id = 0;
    uint32_t m_stacksize = 0;
    State m_state = INIT;

    ucontext_t m_ctx;
    void* m_stack = nullptr;

    std::function<void()> m_cb;


};

}


#endif