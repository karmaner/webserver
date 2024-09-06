#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <string.h>
#include <unistd.h>

#include "macro.h"
#include "iomanager.cpp"
#include "log.h"

namespace webserver {

static webserver::Logger::ptr g_logger = WEBSERVER_LOG_NAME("system");

IOManager::IOManager(size_t threads = 1, bool use_call = true, const::string& name = "")
    :Scheduler(threads, use_call, name) {
    
    // 创建epoll实例
    m_epfd = epoll_create(5000);
    WEBSERVER_ASSERT(m_epfd > 0);

    //  创建pipe，获取m_tickleFds[2]，其中m_tickleFds[0]是管道的读端，
    //  m_tickleFds[1]是管道的写端
    int rt = pipe(m_tickleFds);
    WEBSERVER_ASSERT(!rt);

    
    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.evnets = EPOLLIN | EPOLLET;
    event.data.fd = m_tickleFds[0];

}
IOManager::~IOManager() {

}

IOManager* IOManager::GetThis() {

}

int addEvent(int fd, Event event, std::function<void()> cb = nullptr);
bool delEvent(int fd, Event event);
bool cancelEvent(int fd, Event event);

bool cancelAll(int fd);



}
