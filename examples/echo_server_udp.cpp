#include "src/basic/socket.h"
#include "src/basic/log.h"
#include "src/basic/iomanager.h"

static webserver::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();

void run() {
    webserver::IPAddress::ptr addr = webserver::Address::LookupAnyIPAddress("0.0.0.0:8050");
    webserver::Socket::ptr sock = webserver::Socket::CreateUDP(addr);
    if(sock->bind(addr)) {
        WEBSERVER_LOG_INFO(g_logger) << "udp bind : " << *addr;
    } else {
        WEBSERVER_LOG_ERROR(g_logger) << "udp bind : " << *addr << " fail";
        return;
    }
    while(true) {
        char buff[1024];
        webserver::Address::ptr from(new webserver::IPv4Address);
        int len = sock->recvFrom(buff, 1024, from);
        if(len > 0) {
            buff[len] = '\0';
            WEBSERVER_LOG_INFO(g_logger) << "recv: " << buff << " from: " << *from;
            len = sock->sendTo(buff, len, from);
            if(len < 0) {
                WEBSERVER_LOG_INFO(g_logger) << "send: " << buff << " to: " << *from
                    << " error=" << len;
            }
        }
    }
}

int main(int argc, char** argv) {
    webserver::IOManager iom(1);
    iom.schedule(run);
    return 0;
}
