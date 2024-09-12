#ifndef __SRC_BASIC_TCP_SERVER_H__
#define __SRC_BASIC_TCP_SERVER_H__

#include <memory>
#include <functional>

#include "address.h"
#include "iomanager.h"
#include "socket.h"
#include "noncopyable.h"

namespace webserver {

class TcpServer : public std::enable_shared_from_this<TcpServer>
                    , Noncopyable {
public:
    typedef std::shared_ptr<TcpServer> ptr;
    TcpServer(webserver::IOManager* woker = webserver::IOManager::GetThis()
                ,webserver::IOManager* accept_woker = webserver::IOManager::GetThis());
    virtual ~TcpServer();

    virtual bool bind(webserver::Address::ptr addr, bool ssl = false);
    virtual bool bind(const std::vector<Address::ptr>& addrs
                        ,std::vector<Address::ptr>& fails
                        ,bool ssl = false);

    bool loadCertificates(const std::string& cert_file, const std::string& key_file);
    
    virtual bool start();
    virtual void stop();

    uint64_t getRecvTimeout() const { return m_recvTimeout;}
    std::string getName() const { return m_name;}
    void setRecvTimeout(uint64_t v) { m_recvTimeout = v;}
    virtual void setName(const std::string& v) { m_name = v;}

    bool isStop() const { return m_isStop;}
protected:
    virtual void handleClient(Socket::ptr client);
    virtual void startAccept(Socket::ptr sock);
private:
    std::vector<Socket::ptr> m_socks;
    IOManager* m_worker;
    IOManager* m_acceptWorker;
    uint64_t m_recvTimeout;
    std::string m_name;
    bool m_isStop;
};

}

#endif