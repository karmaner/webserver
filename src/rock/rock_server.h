#ifndef __SRC_ROCK_SERVER_H__
#define __SRC_ROCK_SERVER_H__

#include "src/rock/rock_stream.h"
#include "src/basic/tcp_server.h"

namespace webserver {

class RockServer : public TcpServer {
public:
    typedef std::shared_ptr<RockServer> ptr;
    RockServer(webserver::IOManager* worker = webserver::IOManager::GetThis()
                ,webserver::IOManager* accept_worker = webserver::IOManager::GetThis());

protected:
    virtual void handleClient(Socket::ptr client) override;
};

}

#endif
