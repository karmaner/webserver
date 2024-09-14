#ifndef __SRC_HTTP_WS_SERVER_H__
#define __SRC_HTTP_WS_SERVER_H__

#include "src/basic/tcp_server.h"
#include "ws_session.h"
#include "ws_servlet.h"

namespace webserver {
namespace http {

class WSServer : public TcpServer {
public:
    typedef std::shared_ptr<WSServer> ptr;

    WSServer(webserver::IOManager* worker = webserver::IOManager::GetThis()
                , webserver::IOManager* io_worker = webserver::IOManager::GetThis()
                , webserver::IOManager* accept_worker = webserver::IOManager::GetThis());

    WSServletDispatch::ptr getWSServletDispatch() const { return m_dispatch;}
    void setWSServletDispatch(WSServletDispatch::ptr v) { m_dispatch = v;}
protected:
    virtual void handleClient(Socket::ptr client) override;
protected:
    WSServletDispatch::ptr m_dispatch;
};

}
}

#endif