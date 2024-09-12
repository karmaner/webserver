#ifndef __SRC_HTTP_HTTP_SERVER_H__
#define __SRC_HTTP_HTTP_SERVER_H__

#include "src/basic/tcp_server.h"
#include "servlet.h"

namespace webserver {
namespace http {

class HttpServer : public TcpServer {
public:
    typedef std::shared_ptr<HttpServer> ptr;
    HttpServer(bool keepalive = false
                ,webserver::IOManager* worker = webserver::IOManager::GetThis()
                ,webserver::IOManager* accept_worker = webserver::IOManager::GetThis());

    ServletDispatch::ptr getServletDispatch() const { return m_dispatch;}
    void setServletDispatch(ServletDispatch::ptr v) { m_dispatch = v;}
    virtual void setName(const std::string& v) override;
protected:
    virtual void handleClient(Socket::ptr client) override;
private:
    bool m_isKeepalive;
    ServletDispatch::ptr m_dispatch;
};

}
}

#endif