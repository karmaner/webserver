#include "src/basic/address.h"
#include "src/basic/log.h"

webserver::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();

void test() {
    std::vector<webserver::Address::ptr> addrs;

    WEBSERVER_LOG_INFO(g_logger) << "begin";
    //bool v = webserver::Address::Lookup(addrs, "localhost:3080");
    //bool v = webserver::Address::Lookup(addrs, "www.baidu.com", AF_INET);
    bool v = webserver::Address::Lookup(addrs, "www.bilibili.com", AF_INET);
    WEBSERVER_LOG_INFO(g_logger) << "end";
    if(!v) {
        WEBSERVER_LOG_ERROR(g_logger) << "lookup fail";
        return;
    }

    for(size_t i = 0; i < addrs.size(); ++i) {
        WEBSERVER_LOG_INFO(g_logger) << i << " - " << addrs[i]->toString();
    }

    auto addr = webserver::Address::LookupAny("localhost:4080");
    if(addr) {
        WEBSERVER_LOG_INFO(g_logger) << *addr;
    } else {
        WEBSERVER_LOG_ERROR(g_logger) << "error";
    }
}

void test_iface() {
    std::multimap<std::string, std::pair<webserver::Address::ptr, uint32_t> > results;

    bool v = webserver::Address::GetInterfaceAddresses(results);
    if(!v) {
        WEBSERVER_LOG_ERROR(g_logger) << "GetInterfaceAddresses fail";
        return;
    }

    for(auto& i: results) {
        WEBSERVER_LOG_INFO(g_logger) << i.first << " - " << i.second.first->toString() << " - "
            << i.second.second;
    }
}

void test_ipv4() {
    //auto addr = webserver::IPAddress::Create("www.webserver.top");
    auto addr = webserver::IPAddress::Create("127.0.0.8");
    if(addr) {
        WEBSERVER_LOG_INFO(g_logger) << addr->toString();
    }
}

int main(int argc, char** argv) {
    //test_ipv4();
    //test_iface();
    test();
    return 0;
}
