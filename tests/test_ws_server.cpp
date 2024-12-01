#include "src/http/ws_server.h"
#include "src/basic/log.h"

static webserver::Logger::ptr g_logger = LOG_ROOT();

void run() {
    webserver::http::WSServer::ptr server(new webserver::http::WSServer);
    webserver::Address::ptr addr = webserver::Address::LookupAnyIPAddress("0.0.0.0:8020");
    if(!addr) {
        LOG_ERROR(g_logger) << "get address error";
        return;
    }
    auto fun = [](webserver::http::HttpRequest::ptr header
                    ,webserver::http::WSFrameMessage::ptr msg
                    ,webserver::http::WSSession::ptr session) {
        session->sendMessage(msg);
        return 0;
    };


    server->getWSServletDispatch()->addServlet("/webserver", fun);
    while(!server->bind(addr)) {
        LOG_ERROR(g_logger) << "bind " << *addr << " fail";
        sleep(1);
    }
    server->start();
}

int main(int argc, char* argv[]) {
    webserver::IOManager iom(2);
    iom.schedule(run);
    return 0;
}
