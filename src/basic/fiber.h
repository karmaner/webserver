#ifndef __SRC_FIBER_H__
#define __SRC_FIBER_H__

#include <memory>
#include <functional>
#include <ucontext.h>

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
        /**
     * @brief 构造函数
     * @param[in] cb 协程执行的函数
     * @param[in] stacksize 协程栈大小
     * @param[in] use_caller 是否在MainFiber上调度
     */
    Fiber(std::function<void()> cb, size_t stacksize = 0, bool use_caller = false);
    ~Fiber();

    // 重置协程函数， 并重置状态
    void reset(std::function<void()> cb);
    // 切换当前协程
    void swapIn();  // 线程内容切换到协程执行
    // 切换到后台执行
    void swapOut(); // 保存协程的上下文并切换到其他任务

    // 线程切换到前台
    void call();    // 切换当前线程到前台执行协程
    // 线程切换到后台
    void back();    // 将当前线程切换到后台执行其他任务

    uint64_t getId() const { return m_id; }

    void setState(State s) { m_state = s; }
    State getState() const { return m_state; }

public:
    // 设置当前线程的运行协程
    static void SetThis(Fiber* f);  // 设置当前线程的运行协程为 f。通常用于在协程开始执行时进行设置
    // 返回当前所在的协程
    static Fiber::ptr GetThis();    // 获取当前线程正在执行的协程
    
    // 将当前协程切换到后台,并设置为READY状态
    static void YieldToReady();     // 将当前协程切换到后台并设置为 READY 状态，通常用于在协程中暂停执行，并将其放入待执行队列中。
    // 将当前协程切换到后台,并设置为HOLD状态
    static void YieldToHold();      // 将当前协程切换到后台并设置为 HOLD 状态，通常用于在协程中暂停执行，但保持其当前状态。

    static uint64_t TotalFibers();
    static uint64_t GetFiberId();
    
    static void MainFunc();         // 协程的主函数。通常包含协程的主要执行逻辑。
    static void callerMainFunc();   // 处理协程的调用者逻辑 

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