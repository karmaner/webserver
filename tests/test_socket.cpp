#include "src/basic/socket.h"
#include "src/webserver.h"
#include "src/basic/iomanager.h"

static webserver::Logger::ptr g_looger = WEBSERVER_LOG_ROOT();

void test_socket() {
    //std::vector<webserver::Address::ptr> addrs;
    //webserver::Address::Lookup(addrs, "www.baidu.com", AF_INET);
    //webserver::IPAddress::ptr addr;
    //for(auto& i : addrs) {
    //    WEBSERVER_LOG_INFO(g_looger) << i->toString();
    //    addr = std::dynamic_pointer_cast<webserver::IPAddress>(i);
    //    if(addr) {
    //        break;
    //    }
    //}
    webserver::IPAddress::ptr addr = webserver::Address::LookupAnyIPAddress("www.github.com");
    if(addr) {
        WEBSERVER_LOG_INFO(g_looger) << "get address: " << addr->toString();
    } else {
        WEBSERVER_LOG_ERROR(g_looger) << "get address fail";
        return;
    }

    webserver::Socket::ptr sock = webserver::Socket::CreateTCP(addr);
    addr->setPort(80);
    WEBSERVER_LOG_INFO(g_looger) << "addr=" << addr->toString();
    if(!sock->connect(addr)) {
        WEBSERVER_LOG_ERROR(g_looger) << "connect " << addr->toString() << " fail";
        return;
    } else {
        WEBSERVER_LOG_INFO(g_looger) << "connect " << addr->toString() << " connected";
    }

    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    if(rt <= 0) {
        WEBSERVER_LOG_INFO(g_looger) << "send fail rt=" << rt;
        return;
    }

    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());

    if(rt <= 0) {
        WEBSERVER_LOG_INFO(g_looger) << "recv fail rt=" << rt;
        return;
    }

    buffs.resize(rt);
    WEBSERVER_LOG_INFO(g_looger) << buffs;
}

void test2() {
    webserver::IPAddress::ptr addr = webserver::Address::LookupAnyIPAddress("www.bilibili.com:80");
    if(addr) {
        WEBSERVER_LOG_INFO(g_looger) << "get address: " << addr->toString();
    } else {
        WEBSERVER_LOG_ERROR(g_looger) << "get address fail";
        return;
    }

    webserver::Socket::ptr sock = webserver::Socket::CreateTCP(addr);
    if(!sock->connect(addr)) {
        WEBSERVER_LOG_ERROR(g_looger) << "connect " << addr->toString() << " fail";
        return;
    } else {
        WEBSERVER_LOG_INFO(g_looger) << "connect " << addr->toString() << " connected";
    }

    uint64_t ts = webserver::GetCurrentUS();
    for(size_t i = 0; i < 10000000000ul; ++i) {
        if(int err = sock->getError()) {
            WEBSERVER_LOG_INFO(g_looger) << "err=" << err << " errstr=" << strerror(err);
            break;
        }

        //struct tcp_info tcp_info;
        //if(!sock->getOption(IPPROTO_TCP, TCP_INFO, tcp_info)) {
        //    WEBSERVER_LOG_INFO(g_looger) << "err";
        //    break;
        //}
        //if(tcp_info.tcpi_state != TCP_ESTABLISHED) {
        //    WEBSERVER_LOG_INFO(g_looger)
        //            << " state=" << (int)tcp_info.tcpi_state;
        //    break;
        //}
        static int batch = 10000000;
        if(i && (i % batch) == 0) {
            uint64_t ts2 = webserver::GetCurrentUS();
            WEBSERVER_LOG_INFO(g_looger) << "i=" << i << " used: " << ((ts2 - ts) * 1.0 / batch) << " us";
            ts = ts2;
        }
    }
}


int main(int argc, char* argv[]) {
    webserver::IOManager iom;
    // iom.schedule(&test_socket);
    iom.schedule(&test2);
    return 0;
}