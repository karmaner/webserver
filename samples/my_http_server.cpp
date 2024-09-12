#include "src/http/http_server.h"
#include "src/basic/log.h"

webserver::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();
webserver::IOManager::ptr worker;
void run() {
    g_logger->setLevel(webserver::LogLevel::INFO);
    webserver::Address::ptr addr = webserver::Address::LookupAnyIPAddress("0.0.0.0:8020");
    if(!addr) {
        WEBSERVER_LOG_ERROR(g_logger) << "get address error";
        return;
    }

    webserver::http::HttpServer::ptr http_server(new webserver::http::HttpServer(true, worker.get()));
    //webserver::http::HttpServer::ptr http_server(new webserver::http::HttpServer(true));
    while(!http_server->bind(addr)) {
        WEBSERVER_LOG_ERROR(g_logger) << "bind " << *addr << " fail";
        sleep(1);
    }

    http_server->start();
}

int main(int argc, char** argv) {
    webserver::IOManager iom(1);
    worker.reset(new webserver::IOManager(4, false));
    iom.schedule(run);
    return 0;
}
