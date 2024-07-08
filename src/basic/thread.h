#ifndef __SRC_THREAD_H__
#define __SRC_THREAD_H__

#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <string>
#include <thread>

namespace webserver
{

class Semaphore : private boost::noncopyable
{
  public:
  private:
    sem_t m_semaphore;
};

class Thread : private boost::noncopyable
{
  public:
    typedef std::shared_ptr<Thread> ptr;

    Thread(std::function<void()> cb, const std::string &name);
    ~Thread();

    pid_t getId() const
    {
        return m_id;
    }
    const std::string &getName() const
    {
        return m_name;
    }

    void join();

    static Thread *GetThis();
    static const std::string &GetName();
    static void SetName(const std::string &name);

  private:
    static void *run(void *arg);

  private:
    pid_t m_id = -1;
    pthread_t m_thread = 0;
    std::string m_name;

    std::function<void()> m_cb;
};

} // namespace webserver
#endif