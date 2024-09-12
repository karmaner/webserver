#include <iostream>
#include "src/http/http_connection.h"
#include "src/basic/log.h"
#include "src/basic/iomanager.h"
#include <fstream>

static webserver::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();

void test_pool() {
    webserver::http::HttpConnectionPool::ptr pool(new webserver::http::HttpConnectionPool(
                "www.bilibili.com", "", 80, 10, 1000 * 30, 5));

    webserver::IOManager::GetThis()->addTimer(1000, [pool](){
            auto r = pool->doGet("/", 300);
            WEBSERVER_LOG_INFO(g_logger) << r->toString();
    }, true);
}

void run() {
    webserver::Address::ptr addr = webserver::Address::LookupAnyIPAddress("www.sylar.top:80");
    if(!addr) {
        WEBSERVER_LOG_INFO(g_logger) << "get addr error";
        return;
    }

    webserver::Socket::ptr sock = webserver::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    if(!rt) {
        WEBSERVER_LOG_INFO(g_logger) << "connect " << *addr << " failed";
        return;
    }

    webserver::http::HttpConnection::ptr conn(new webserver::http::HttpConnection(sock));
    webserver::http::HttpRequest::ptr req(new webserver::http::HttpRequest);
    req->setPath("/blog/");
    req->setHeader("host", "www.sylar.top");
    WEBSERVER_LOG_INFO(g_logger) << "req:" << std::endl
        << *req;

    conn->sendRequest(req);
    auto rsp = conn->recvResponse();

    if(!rsp) {
        WEBSERVER_LOG_INFO(g_logger) << "recv response error";
        return;
    }
    WEBSERVER_LOG_INFO(g_logger) << "rsp:" << std::endl
        << *rsp;

    std::ofstream ofs("rsp.dat");
    ofs << *rsp;

    
    WEBSERVER_LOG_INFO(g_logger) << "=========================";

    auto r = webserver::http::HttpConnection::DoGet("http://www.sylar.top/blog/", 300);
    WEBSERVER_LOG_INFO(g_logger) << "result=" << r->result
        << " error=" << r->error
        << " rsp=" << (r->response ? r->response->toString() : "");

    WEBSERVER_LOG_INFO(g_logger) << "=========================";
    test_pool();
}

int main(int argc, char** argv) {
    webserver::IOManager iom(2);
    iom.schedule(run);
    return 0;
}