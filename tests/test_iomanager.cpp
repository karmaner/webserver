#include "src/webserver.h"
#include "src/basic/iomanager.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>

webserver::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();

int sock = 0;

void test_fiber() {
    WEBSERVER_LOG_INFO(g_logger) << "test_fiber sock=" << sock;

    //sleep(3);

    //close(sock);
    //webserver::IOManager::GetThis()->cancelAll(sock);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "115.239.210.27", &addr.sin_addr.s_addr);

    if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))) {
    } else if(errno == EINPROGRESS) {
        WEBSERVER_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        webserver::IOManager::GetThis()->addEvent(sock, webserver::IOManager::READ, [](){
            WEBSERVER_LOG_INFO(g_logger) << "read callback";
        });
        webserver::IOManager::GetThis()->addEvent(sock, webserver::IOManager::WRITE, [](){
            WEBSERVER_LOG_INFO(g_logger) << "write callback";
            //close(sock);
            webserver::IOManager::GetThis()->cancelEvent(sock, webserver::IOManager::READ);
            close(sock);
        });
    } else {
        WEBSERVER_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }

}

void test1() {
    std::cout << "EPOLLIN=" << EPOLLIN
                << " EPOLLOUT=" << EPOLLOUT << std::endl;
    webserver::IOManager iom(2, false);
    iom.schedule(&test_fiber);
}

int main(int argc, char* argv[]) {

    test1();
    return 0;
}