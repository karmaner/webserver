#include "src/webserver.h"
#include "src/ns/ns_protocol.h"
#include "src/ns/ns_client.h"

static webserver::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();

int type = 0;

void run() {
    g_logger->setLevel(webserver::LogLevel::INFO);
    auto addr = webserver::IPAddress::Create("127.0.0.1", 8072);
    //if(!conn->connect(addr)) {
    //    WEBSERVER_LOG_ERROR(g_logger) << "connect to: " << *addr << " fail";
    //    return;
    //}
    if(type == 0) {
        for(int i = 0; i < 5000; ++i) {
            webserver::RockConnection::ptr conn(new webserver::RockConnection);
            conn->connect(addr);
            webserver::IOManager::GetThis()->addTimer(3000, [conn, i](){
                    webserver::RockRequest::ptr req(new webserver::RockRequest);
                    req->setCmd((int)webserver::ns::NSCommand::REGISTER);
                    auto rinfo = std::make_shared<webserver::ns::RegisterRequest>();
                    auto info = rinfo->add_infos();
                    info->set_domain(std::to_string(rand() % 2) + "domain.com");
                    info->add_cmds(rand() % 2 + 100);
                    info->add_cmds(rand() % 2 + 200);
                    info->mutable_node()->set_ip("127.0.0.1");
                    info->mutable_node()->set_port(1000 + i);
                    info->mutable_node()->set_weight(100);
                    req->setAsPB(*rinfo);

                    auto rt = conn->request(req, 100);
                    WEBSERVER_LOG_INFO(g_logger) << "[result="
                        << rt->result << " response="
                        << (rt->response ? rt->response->toString() : "null")
                        << "]";
            }, true);
            conn->start();
        }
    } else {
        for(int i = 0; i < 1000; ++i) {
            webserver::ns::NSClient::ptr nsclient(new webserver::ns::NSClient);
            nsclient->init();
            nsclient->addQueryDomain(std::to_string(i % 2) + "domain.com");
            nsclient->connect(addr);
            nsclient->start();
            WEBSERVER_LOG_INFO(g_logger) << "NSClient start: i=" << i;

            if(i == 0) {
                //webserver::IOManager::GetThis()->addTimer(1000, [nsclient](){
                //    auto domains = nsclient->getDomains();
                //    domains->dump(std::cout, "    ");
                //}, true);
            }
        }

        //conn->setConnectCb([](webserver::AsyncSocketStream::ptr ss) {
        //    auto conn = std::dynamic_pointer_cast<webserver::RockConnection>(ss);
        //    webserver::RockRequest::ptr req(new webserver::RockRequest);
        //    req->setCmd((int)webserver::ns::NSCommand::QUERY);
        //    auto rinfo = std::make_shared<webserver::ns::QueryRequest>();
        //    rinfo->add_domains("0domain.com");
        //    req->setAsPB(*rinfo);
        //    auto rt = conn->request(req, 1000);
        //    WEBSERVER_LOG_INFO(g_logger) << "[result="
        //        << rt->result << " response="
        //        << (rt->response ? rt->response->toString() : "null")
        //        << "]";
        //    return true;
        //});

        //conn->setNotifyHandler([](webserver::RockNotify::ptr nty,webserver::RockStream::ptr stream){
        //        auto nm = nty->getAsPB<webserver::ns::NotifyMessage>();
        //        if(!nm) {
        //            WEBSERVER_LOG_ERROR(g_logger) << "invalid notify message";
        //            return true;
        //        }
        //        WEBSERVER_LOG_INFO(g_logger) << webserver::PBToJsonString(*nm);
        //        return true;
        //});
    }
}

int main(int argc, char** argv) {
    if(argc > 1) {
        type = 1;
    }
    webserver::IOManager iom(5);
    iom.schedule(run);
    return 0;
}
