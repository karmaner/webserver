#include <atomic>

#include "fiber.h"
#include "config.h"
#include "log.h"
#include "macro.h"
#include "scheduler.h"


namespace webserver {

static Logger::ptr g_logger = WEBSERVER_LOG_NAME("system");

static std::atomic<uint64_t> s_fiber_id {0};                              // 用于生成协程ID
static std::atomic<uint64_t> s_fiber_count {0};                           // 用于统计协程数

static thread_local Fiber* t_fiber = nullptr;                             // 当前线程正在运行的协程
static thread_local Fiber::ptr t_threadFiber = nullptr;                   // 当前线程的主协程

static ConfigVar<uint32_t>::ptr g_fiber_stack_size =
    Config::Lookup<uint32_t>("fiber.stack_size", 1024 * 1024, "fiber stack size");  // new一个配置

// 内存分配器
class MallocStackAllocator {
public:
    static void* Alloc(size_t size) {
        return malloc(size);
    }

    static void Dealloc(void* vp, size_t size) {
        return free(vp);
    } 
};

using StackAllocator = MallocStackAllocator;

// 协程上下文 给到t_fiber
Fiber::Fiber() {
    m_state = EXEC;
    SetThis(this);

    if(getcontext(&m_ctx)) {
        WEBSERVER_ASSERT2(false, "getcontext");
    }

    ++s_fiber_count;
    WEBSERVER_LOG_DEBUG(g_logger) << "FIber::Fiber main";
}

// 创建协程  线程上下文给到协程
Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool use_caller) 
    :m_id(++s_fiber_id)
    ,m_cb(cb) {

    ++s_fiber_count;
    m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();

    m_stack = StackAllocator::Alloc(m_stacksize);
    if(getcontext(&m_ctx)) {
        WEBSERVER_ASSERT2(false, "gencontext");
    }
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    if(!use_caller) {
        makecontext(&m_ctx, &Fiber::MainFunc, 0);
    } else {
        makecontext(&m_ctx, &Fiber::callerMainFunc, 0);
    }

    WEBSERVER_LOG_DEBUG(g_logger) << "Fiber::Fiber id= " << m_id;
}


Fiber::~Fiber() {
    --s_fiber_count;
    if(m_stack) {
        WEBSERVER_ASSERT((m_state == TERM
        || m_state == EXCEPT
        || m_state == INIT));

        StackAllocator::Dealloc(m_stack, m_stacksize);
    } else {
        WEBSERVER_ASSERT(!m_cb);
        WEBSERVER_ASSERT((m_state == EXEC));

        Fiber* cur = t_fiber;
        if(cur == this) {
            SetThis(nullptr);
        }
    }

    WEBSERVER_LOG_DEBUG(g_logger) << "Fiber::~Fiber id=" << m_id
                            << " total=" << s_fiber_count;

}

void Fiber::reset(std::function<void()> cb) {
    WEBSERVER_ASSERT(m_stack);
    WEBSERVER_ASSERT((m_state == TERM
            || m_state == EXCEPT
            || m_state == INIT));
    m_cb = cb;

    if(getcontext(&m_ctx)) {
        WEBSERVER_ASSERT2(false, "getcontext");
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
    m_state = INIT;
}

void Fiber::swapIn() {
    SetThis(this);
    WEBSERVER_ASSERT((m_state != EXEC));
    m_state = EXEC;
    if(swapcontext(&Scheduler::GetMainFiber()->m_ctx, &m_ctx)) {
        WEBSERVER_ASSERT2(false, "swapcontext");
    }
}
void Fiber::swapOut() {
    SetThis(t_threadFiber.get());

    if(swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx)) {
        WEBSERVER_ASSERT2(false, "swapcontext");
    }
}

void Fiber::call() {
    SetThis(this);
    m_state = EXEC;
    WEBSERVER_LOG_ERROR(g_logger) << getId();
    if(swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {
        WEBSERVER_ASSERT2(false, "swapcontext"); 
    }
}

void Fiber::back() {
    SetThis(this);
    if(swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
        WEBSERVER_ASSERT2(false, "swapcontext");
    }
}

void Fiber::SetThis(Fiber* f) {
    t_fiber = f;
}

Fiber::ptr Fiber::GetThis() {
    if(t_fiber) {
        return t_fiber->shared_from_this();
    }

    Fiber::ptr main_fiber(new Fiber);
    WEBSERVER_ASSERT((t_fiber == main_fiber.get()));
    t_threadFiber = main_fiber;
    return t_fiber->shared_from_this();
}

void Fiber::YieldToReady() {
    Fiber::ptr cur = GetThis();
    WEBSERVER_ASSERT((cur->m_state == EXEC));
    cur->m_state = READY;
    cur->swapOut();
}
void Fiber::YieldToHold() {
    Fiber::ptr cur = GetThis();
    WEBSERVER_ASSERT((cur->m_state == EXEC));
    cur->swapOut();
}

uint64_t Fiber::TotalFibers() {
    return s_fiber_count;
}

uint64_t Fiber::GetFiberId() {
    if (t_fiber) {
        t_fiber->getId();
    }
    return 0;
}

void Fiber::MainFunc() {
    Fiber::ptr cur = GetThis();
    WEBSERVER_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception& ex) {
        cur->m_state = EXCEPT;
        WEBSERVER_LOG_ERROR(g_logger) << "Fiber Execpt: " << ex.what()
            << " Fiber_id=" << cur->getId()
            << std::endl
            << webserver::BacktraceToString();
    } catch (...) {

        cur->m_state = EXCEPT;
        WEBSERVER_LOG_ERROR(g_logger) << "Fiber Except"
            << " fiber_id=" << cur->getId()
            << std::endl
            << webserver::BacktraceToString();
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->swapOut();

    WEBSERVER_ASSERT2(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));

}

void Fiber::callerMainFunc() {
    Fiber::ptr cur = GetThis();
    WEBSERVER_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception& ex) {
        cur->m_state = EXCEPT;
        WEBSERVER_LOG_ERROR(g_logger) << "Fiber Execept: " << ex.what()
                                    << " fiber_id=" << cur->getId()
                                    << std::endl
                                    << webserver::BacktraceToString();
    } catch (...) {
        cur->m_state = EXCEPT;
        WEBSERVER_LOG_ERROR(g_logger) << "Fiber Except"
            << " fiber_id=" << cur->getId()
            << std::endl
            << webserver::BacktraceToString();
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->back();
    
    WEBSERVER_ASSERT2(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));
}

}