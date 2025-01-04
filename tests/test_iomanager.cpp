#include "src/webserver.h"
#include "src/basic/iomanager.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>

webserver::Logger::ptr g_logger = LOG_ROOT();

int sock = 0;

void test_fiber() {
	LOG_INFO(g_logger) << "test_fiber sock=" << sock;

	//sleep(3);

	//close(sock);
	//webserver::IOManager::GetThis()->cancelAll(sock);

	sock = socket(AF_INET, SOCK_STREAM, 0);
  int flags = fcntl(sock, F_GETFL, 0);
  fcntl(sock, F_SETFL, flags | O_NONBLOCK);

	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(3306);
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

	if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))) {
		LOG_INFO(g_logger) << "connect success immediately";
	} else if(errno == EINPROGRESS) {
    webserver::IOManager::GetThis()->addEvent(sock, webserver::IOManager::WRITE, [](){
			int error = 0;
			socklen_t len = sizeof(error);
			
			// 获取实际的连接结果
			if(getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
				LOG_ERROR(g_logger) << "连接失败: 无法获取socket状态";
				close(sock);
				return;
			}
			
			if(error) {
				LOG_ERROR(g_logger) << "连接失败: " << strerror(error);
				close(sock);
				return;
			}
			
			// 连接成功，可以开始收发数据
			LOG_INFO(g_logger) << "连接成功";
    });
	} else {
		LOG_ERROR(g_logger) << "connect error errno=" << errno << " " << strerror(errno);
	}

}

void test1() {
	std::cout << "EPOLLIN=" << EPOLLIN
						<< " EPOLLOUT=" << EPOLLOUT << std::endl;
	webserver::IOManager iom(2, false, "iomanager");
	iom.schedule(&test_fiber);
}

webserver::Timer::ptr s_timer;
void test_timer() {
	webserver::IOManager iom(2);
	s_timer = iom.addTimer(1000, []() {
		static int i = 0;
		LOG_INFO(g_logger) << "hello timer i=" << i;
		if(++i == 3) {
			s_timer->reset(2000, true);
			//s_timer->cancel();
		}
	}, true);
}

int main(int argc, char* argv[]) {
	test1();
	//test_timer();
	return 0;
}

