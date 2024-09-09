#include "src/basic/tcp_server.h"
#include "src/basic/iomanager.h"
#include "src/basic/log.h"

static webserver::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();

void run() {
    auto addr = webserver::Address::LookupAny("0.0.0.0:8033");
    //auto addr2 = webserver::UnixAddress::ptr(new webserver::UnixAddress("/tmp/unix_addr"));
    std::vector<webserver::Address::ptr> addrs;
    addrs.push_back(addr);
    //addrs.push_back(addr2);

    webserver::TcpServer::ptr tcp_server(new webserver::TcpServer);
    std::vector<webserver::Address::ptr> fails;
    while(!tcp_server->bind(addrs, fails)) {
        sleep(2);
    }
    tcp_server->start();

}
int main(int argc, char** argv) {
    webserver::IOManager iom(2);
    iom.schedule(run);
    return 0;
}