#ifndef __SRC_NS_NAME_SERVER_MODULE_H__
#define __SRC_NS_NAME_SERVER_MODULE_H__

#include "src/basic/module.h"
#include "ns_protocol.h"

namespace webserver {
namespace ns {

class NameServerModule;
class NSClientInfo {
friend class NameServerModule;
public:
    typedef std::shared_ptr<NSClientInfo> ptr;
private:
    NSNode::ptr m_node;
    std::map<std::string, std::set<uint32_t> > m_domain2cmds;
};

class NameServerModule : public RockModule {
public:
    typedef std::shared_ptr<NameServerModule> ptr;
    NameServerModule();

    virtual bool handleRockRequest(webserver::RockRequest::ptr request
                        ,webserver::RockResponse::ptr response
                        ,webserver::RockStream::ptr stream) override;
    virtual bool handleRockNotify(webserver::RockNotify::ptr notify
                        ,webserver::RockStream::ptr stream) override;
    virtual bool onConnect(webserver::Stream::ptr stream) override;
    virtual bool onDisconnect(webserver::Stream::ptr stream) override;
    virtual std::string statusString() override;
private:
    bool handleRegister(webserver::RockRequest::ptr request
                        ,webserver::RockResponse::ptr response
                        ,webserver::RockStream::ptr stream);
    bool handleQuery(webserver::RockRequest::ptr request
                        ,webserver::RockResponse::ptr response
                        ,webserver::RockStream::ptr stream);
    bool handleTick(webserver::RockRequest::ptr request
                        ,webserver::RockResponse::ptr response
                        ,webserver::RockStream::ptr stream);

private:
    NSClientInfo::ptr get(webserver::RockStream::ptr rs);
    void set(webserver::RockStream::ptr rs, NSClientInfo::ptr info);

    void setQueryDomain(webserver::RockStream::ptr rs, const std::set<std::string>& ds);

    void doNotify(std::set<std::string>& domains, std::shared_ptr<NotifyMessage> nty);

    std::set<webserver::RockStream::ptr> getStreams(const std::string& domain);
private:
    NSDomainSet::ptr m_domains;

    webserver::RWMutex m_mutex;
    std::map<webserver::RockStream::ptr, NSClientInfo::ptr> m_sessions;

    /// sessoin 关注的域名
    std::map<webserver::RockStream::ptr, std::set<std::string> > m_queryDomains;
    /// 域名对应关注的session
    std::map<std::string, std::set<webserver::RockStream::ptr> > m_domainToSessions;
};

}
}

#endif
