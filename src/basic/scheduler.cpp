#include "scheduler.h"
#include "fiber.h"
#include "macro.h"
#include "log.h"
#include <functional>
// #include "hook.h"

namespace webserver {

static webserver::Logger::ptr g_logger = WEBSERVER_LOG_NAME("system");

static thread_local Scheduler* t_scheduler = nullptr;       // 协程调度器 指针 
static thread_local Fiber* t_fiber = nullptr;     // 主协程

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name)
    : m_name(name) {
    
    WEBSERVER_ASSERT((threads > 0));

    if(use_caller) {
        webserver::Fiber::GetThis();    // 这里面其实创建了当前线程的主协程
        --threads;

        WEBSERVER_ASSERT((GetThis() == nullptr));
        t_scheduler = this;

        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
        webserver::Thread::SetName(m_name);

        t_fiber = m_rootFiber.get();
        m_rootThread = webserver::GetThreadId();
        m_threadIds.push_back(m_rootThread);
    } else {
        m_rootThread = -1;
    }
    m_threadCount = threads;
}

Scheduler::~Scheduler() {
    WEBSERVER_ASSERT(m_stopping);
    if(GetThis() == this) {
        t_scheduler = nullptr;
    }
}

Scheduler* Scheduler::GetThis() {
    return t_scheduler;
}

Fiber* Scheduler::GetMainFiber() {
    return t_fiber;
}

void Scheduler::start() {
    MutexType::Lock lock(m_mutex);
    if(!m_stopping) {
        return;
    }
    m_stopping = false;
    WEBSERVER_ASSERT(m_threads.empty());

    m_threads.resize(m_threadCount);
    for(size_t i = 0; i < m_threadCount; ++i) {
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this),
                                        m_name + "_" + std::to_string(i)));
        m_threadIds.push_back(m_threads[i]->getId());
    }
    lock.unlock();
}

void Scheduler::stop() {
    m_autoStop = true;
    if (m_rootFiber
            && m_threadCount == 0
            && (m_rootFiber->getState() == Fiber::TERM
                || m_rootFiber->getState() == Fiber::INIT)) {
        WEBSERVER_LOG_INFO(g_logger) << this << " stopped";
        m_stopping = true;

        if (stopping()) {
            return;
        }
    }

    if (m_rootThread != -1) {
        WEBSERVER_ASSERT((GetThis() == this));
    } else {
        WEBSERVER_ASSERT((GetThis() != this));
    }

    m_stopping = true;
    for(size_t i = 0; i < m_threadCount; ++i) {
        tickle();   // 唤醒线程
    }

    if(m_rootFiber) {
        tickle();
    }

    if(m_rootFiber) {
        if(!stopping()) {
            // 此时才切换到m_rootFiber这条协程执行
            m_rootFiber->call();
        }
    }

    std::vector<Thread::ptr> thrs;
    {
        MutexType::Lock lock(m_mutex);
        thrs.swap(m_threads);
    }

    for(auto& i : thrs) {
        i->join();
    }

}

void Scheduler::setThis() {
    t_scheduler = this;
}


void Scheduler::run() {
    WEBSERVER_LOG_DEBUG(g_logger) << m_name << " run";

    // set_hook_enable(true);

    setThis();
    if(webserver::GetThreadId() != m_rootThread) {
        t_fiber = Fiber::GetThis().get();
    }

    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
    Fiber::ptr cb_fiber;

    FiberAndThread ft;
    while (true) {
        ft.reset();
        bool tickle_me = false;
        bool is_active = false;
        {   // 取出任务
            MutexType::Lock lock(m_mutex);
            auto it = m_fibers.begin();
            while (it != m_fibers.end()) {
                if (it ->thread != -1 && it->thread != webserver::GetThreadId()) {
                    ++it;
                    tickle_me = true;
                    continue;
                }

                WEBSERVER_ASSERT(it->fiber || it->cb);

                if (it->fiber && it->fiber->getState() == Fiber::EXEC) {
                    ++it;
                    continue;
                }

                ft = *it;
                m_fibers.erase(it++);
                ++m_activeThreadCount;
                is_active = true;
                break;
            }
            tickle_me |= it != m_fibers.end();
        }

        if(tickle_me) tickle();

        if(ft.fiber && (ft.fiber->getState() != Fiber::TERM
                        && ft.fiber->getState() != Fiber::EXCEPT)) {
            ft.fiber->swapIn();         // 切进去执行该协程
            --m_activeThreadCount;      // 等该协程出来的时候，活跃线程数便可以减1

            // 该协程出来后，如果还是READY状态，则再把它进入到任务队列中
            if(ft.fiber->getState() == Fiber::READY) {
                schedule(ft.fiber);
            } else if(ft.fiber->getState() != Fiber::TERM
                    && ft.fiber->getState() != Fiber::EXCEPT) {
                ft.fiber->setState(Fiber::HOLD);
            }
            ft.reset();
        } else if(ft.cb) {      // 任务包装的是函数对象
            if(cb_fiber) {
                cb_fiber->reset(ft.cb);
            } else {
                cb_fiber.reset(new Fiber(ft.cb));
            }
            ft.reset();
            cb_fiber->swapIn();
            --m_activeThreadCount;
            
            if (cb_fiber->getState() == Fiber::READY) {
                schedule(cb_fiber);
                cb_fiber.reset();
            } else if(cb_fiber->getState() == Fiber::EXCEPT
                    || cb_fiber->getState() == Fiber::TERM) {
                cb_fiber->reset(nullptr);
            } else {
                cb_fiber->setState(Fiber::HOLD);
                cb_fiber.reset();
            }
        } else {        // 无任务
            if(is_active) {
                --m_activeThreadCount;
                continue;
            }

            if(idle_fiber->getState() == Fiber::TERM) {
                WEBSERVER_LOG_INFO(g_logger) << "idle fiber term";
                break;
            }

            ++m_idleThreadCount;
            idle_fiber->swapIn();       // 执行idle协程，其实是在忙等
            --m_idleThreadCount;
            if(idle_fiber->getState() != Fiber::TERM
                    && idle_fiber->getState() != Fiber::EXCEPT) {
                idle_fiber->setState(Fiber::HOLD);
            }
        }
    }
}

// 子类实现
void Scheduler::tickle() {
    WEBSERVER_LOG_INFO(g_logger) << "tickle";
}

bool Scheduler::stopping() {
    MutexType::Lock lock(m_mutex);
    return m_autoStop && m_stopping
        && m_fibers.empty() && m_activeThreadCount == 0;
}

void Scheduler::idle() {
    WEBSERVER_LOG_INFO(g_logger) << "idle";
    while(!stopping()) {
        webserver::Fiber::YieldToHold();
    }
}


void Scheduler::switchTo(int thread) {
    WEBSERVER_ASSERT((Scheduler::GetThis() != nullptr));
    if(Scheduler::GetThis() == this) {
        if(thread == -1 || thread == webserver::GetThreadId()) return;
    }

    schedule(Fiber::GetThis(), thread);
    Fiber::YieldToHold();
}

std::ostream& Scheduler::dump(std::ostream& os) {
    os << "[Scheduler name=" << m_name
        << " size=" << m_threadCount
        << " active_count=" << m_activeThreadCount
        << " idle_count=" << m_idleThreadCount
        << " stopping=" << m_stopping
        << " ]" << std::endl << "    ";

    for (size_t i = 0; i < m_threadIds.size(); ++i) {
        if(i) {
            os << ", ";
        }
        os << m_threadIds[i];
    }
    return os;
}


SchedulerSwitcher::SchedulerSwitcher(Scheduler* target) {
    m_caller = Scheduler::GetThis();
    if (target) {
        target->switchTo();
    }
}

SchedulerSwitcher::~SchedulerSwitcher() {
    if(m_caller) {
        m_caller->switchTo();
    }
}

} // end of namespace
