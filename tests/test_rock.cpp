#include "src/webserver.h"
#include "src/rock/rock_stream.h"

static webserver::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();

webserver::RockConnection::ptr conn(new webserver::RockConnection);
void run() {
    conn->setAutoConnect(true);
    webserver::Address::ptr addr = webserver::Address::LookupAny("127.0.0.1:8061");
    if(!conn->connect(addr)) {
        WEBSERVER_LOG_INFO(g_logger) << "connect " << *addr << " false";
    }
    conn->start();

    webserver::IOManager::GetThis()->addTimer(1000, [](){
        webserver::RockRequest::ptr req(new webserver::RockRequest);
        static uint32_t s_sn = 0;
        req->setSn(++s_sn);
        req->setCmd(100);
        req->setBody("hello world sn=" + std::to_string(s_sn));

        auto rsp = conn->request(req, 300);
        if(rsp->response) {
            WEBSERVER_LOG_INFO(g_logger) << rsp->response->toString();
        } else {
            WEBSERVER_LOG_INFO(g_logger) << "error result=" << rsp->result;
        }
    }, true);
}

int main(int argc, char** argv) {
    webserver::IOManager iom(1);
    iom.schedule(run);
    return 0;
}