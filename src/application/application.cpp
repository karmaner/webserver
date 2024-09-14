#include "application.h"

#include <unistd.h>
#include <signal.h>

#include "src/basic/tcp_server.h"
#include "src/basic/daemon.h"
#include "src/basic/config.h"
#include "src/basic/env.h"
#include "src/basic/log.h"
#include "src/basic/module.h"
#include "src/basic/worker.h"
#include "src/http/ws_server.h"
#include "src/rock/rock_server.h"

namespace webserver {

static webserver::Logger::ptr g_logger = WEBSERVER_LOG_NAME("system");

static webserver::ConfigVar<std::string>::ptr g_server_work_path =
    webserver::Config::Lookup("server.work_path"
            ,std::string("/apps/work/webserver")
            , "server work path");

static webserver::ConfigVar<std::string>::ptr g_server_pid_file =
    webserver::Config::Lookup("server.pid_file"
            ,std::string("webserver.pid")
            , "server pid file");

static webserver::ConfigVar<std::vector<TcpServerConf> >::ptr g_servers_conf
    = webserver::Config::Lookup("servers", std::vector<TcpServerConf>(), "http server config");

Application* Application::s_instance = nullptr;

Application::Application() {
    s_instance = this;
}

bool Application::init(int argc, char** argv) {
    m_argc = argc;
    m_argv = argv;

    webserver::EnvMgr::GetInstance()->addHelp("s", "start with the terminal");
    webserver::EnvMgr::GetInstance()->addHelp("d", "run as daemon");
    webserver::EnvMgr::GetInstance()->addHelp("c", "conf path default: ./conf");
    webserver::EnvMgr::GetInstance()->addHelp("p", "print help");

    bool is_print_help = false;
    if(!webserver::EnvMgr::GetInstance()->init(argc, argv)) {
        is_print_help = true;
    }

    if(webserver::EnvMgr::GetInstance()->has("p")) {
        is_print_help = true;
    }

    std::string conf_path = webserver::EnvMgr::GetInstance()->getConfigPath();
    WEBSERVER_LOG_INFO(g_logger) << "load conf path:" << conf_path;
    webserver::Config::LoadFromConfDir(conf_path);

    ModuleMgr::GetInstance()->init();
    std::vector<Module::ptr> modules;
    ModuleMgr::GetInstance()->listAll(modules);

    for(auto i : modules) {
        i->onBeforeArgsParse(argc, argv);
    }

    if(is_print_help) {
        webserver::EnvMgr::GetInstance()->printHelp();
        return false;
    }

    for(auto i : modules) {
        i->onAfterArgsParse(argc, argv);
    }
    modules.clear();

    int run_type = 0;
    if(webserver::EnvMgr::GetInstance()->has("s")) {
        run_type = 1;
    }
    if(webserver::EnvMgr::GetInstance()->has("d")) {
        run_type = 2;
    }

    if(run_type == 0) {
        webserver::EnvMgr::GetInstance()->printHelp();
        return false;
    }

    std::string pidfile = g_server_work_path->getValue()
                                + "/" + g_server_pid_file->getValue();
    if(webserver::FSUtil::IsRunningPidfile(pidfile)) {
        WEBSERVER_LOG_ERROR(g_logger) << "server is running:" << pidfile;
        return false;
    }

    if(!webserver::FSUtil::Mkdir(g_server_work_path->getValue())) {
        WEBSERVER_LOG_FATAL(g_logger) << "create work path [" << g_server_work_path->getValue()
            << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

bool Application::run() {
    bool is_daemon = webserver::EnvMgr::GetInstance()->has("d");
    return start_daemon(m_argc, m_argv,
            std::bind(&Application::main, this, std::placeholders::_1,
                std::placeholders::_2), is_daemon);
}

int Application::main(int argc, char** argv) {
    signal(SIGPIPE, SIG_IGN);
    WEBSERVER_LOG_INFO(g_logger) << "main";
    std::string conf_path = webserver::EnvMgr::GetInstance()->getConfigPath();
    webserver::Config::LoadFromConfDir(conf_path, true);
    {
        std::string pidfile = g_server_work_path->getValue()
                                    + "/" + g_server_pid_file->getValue();
        std::ofstream ofs(pidfile);
        if(!ofs) {
            WEBSERVER_LOG_ERROR(g_logger) << "open pidfile " << pidfile << " failed";
            return false;
        }
        ofs << getpid();
    }

    m_mainIOManager.reset(new webserver::IOManager(1, true, "main"));
    m_mainIOManager->schedule(std::bind(&Application::run_fiber, this));
    m_mainIOManager->addTimer(2000, [](){
            //WEBSERVER_LOG_INFO(g_logger) << "hello";
    }, true);
    m_mainIOManager->stop();
    return 0;
}

int Application::run_fiber() {
    std::vector<Module::ptr> modules;
    ModuleMgr::GetInstance()->listAll(modules);
    bool has_error = false;
    for(auto& i : modules) {
        if(!i->onLoad()) {
            WEBSERVER_LOG_ERROR(g_logger) << "module name="
                << i->getName() << " version=" << i->getVersion()
                << " filename=" << i->getFilename();
            has_error = true;
        }
    }
    if(has_error) {
        _exit(0);
    }
    webserver::WorkerMgr::GetInstance()->init();
    auto http_confs = g_servers_conf->getValue();
    for(auto& i : http_confs) {
        WEBSERVER_LOG_DEBUG(g_logger) << std::endl << LexicalCast<TcpServerConf, std::string>()(i);

        std::vector<Address::ptr> address;
        for(auto& a : i.address) {
            size_t pos = a.find(":");
            if(pos == std::string::npos) {
                //WEBSERVER_LOG_ERROR(g_logger) << "invalid address: " << a;
                address.push_back(UnixAddress::ptr(new UnixAddress(a)));
                continue;
            }
            int32_t port = atoi(a.substr(pos + 1).c_str());
            //127.0.0.1
            auto addr = webserver::IPAddress::Create(a.substr(0, pos).c_str(), port);
            if(addr) {
                address.push_back(addr);
                continue;
            }
            std::vector<std::pair<Address::ptr, uint32_t> > result;
            if(webserver::Address::GetInterfaceAddresses(result,
                                        a.substr(0, pos))) {
                for(auto& x : result) {
                    auto ipaddr = std::dynamic_pointer_cast<IPAddress>(x.first);
                    if(ipaddr) {
                        ipaddr->setPort(atoi(a.substr(pos + 1).c_str()));
                    }
                    address.push_back(ipaddr);
                }
                continue;
            }

            auto aaddr = webserver::Address::LookupAny(a);
            if(aaddr) {
                address.push_back(aaddr);
                continue;
            }
            WEBSERVER_LOG_ERROR(g_logger) << "invalid address: " << a;
            _exit(0);
        }
        IOManager* accept_worker = webserver::IOManager::GetThis();
        IOManager* process_worker = webserver::IOManager::GetThis();
        if(!i.accept_worker.empty()) {
            accept_worker = webserver::WorkerMgr::GetInstance()->getAsIOManager(i.accept_worker).get();
            if(!accept_worker) {
                WEBSERVER_LOG_ERROR(g_logger) << "accept_worker: " << i.accept_worker
                    << " not exists";
                _exit(0);
            }
        }
        if(!i.process_worker.empty()) {
            process_worker = webserver::WorkerMgr::GetInstance()->getAsIOManager(i.process_worker).get();
            if(!process_worker) {
                WEBSERVER_LOG_ERROR(g_logger) << "process_worker: " << i.process_worker
                    << " not exists";
                _exit(0);
            }
        }

        TcpServer::ptr server;
        if(i.type == "http") {
            server.reset(new webserver::http::HttpServer(i.keepalive,
                            process_worker, accept_worker));
        } else if(i.type == "ws") {
            server.reset(new webserver::http::WSServer(
                            process_worker, accept_worker));
        } else if(i.type == "rock") {
            server.reset(new webserver::RockServer(
                            process_worker, accept_worker));
        } else {
            WEBSERVER_LOG_ERROR(g_logger) << "invalid server type=" << i.type
                << LexicalCast<TcpServerConf, std::string>()(i);
            _exit(0);
        }
        if(!i.name.empty()) {
            server->setName(i.name);
        }
        std::vector<Address::ptr> fails;
        if(!server->bind(address, fails, i.ssl)) {
            for(auto& x : fails) {
                WEBSERVER_LOG_ERROR(g_logger) << "bind address fail:"
                    << *x;
            }
            _exit(0);
        }
        if(i.ssl) {
            if(!server->loadCertificates(i.cert_file, i.key_file)) {
                WEBSERVER_LOG_ERROR(g_logger) << "loadCertificates fail, cert_file="
                    << i.cert_file << " key_file=" << i.key_file;
            }
        }
        server->setConf(i);
        server->start();
        m_servers[i.type].push_back(server);
    }

    for(auto& i : modules) {
        i->onServerReady();
    }
    return 0;
}

bool Application::getServer(const std::string& type, std::vector<TcpServer::ptr>& svrs) {
    auto it = m_servers.find(type);
    if(it == m_servers.end()) {
        return false;
    }
    svrs = it->second;
    return true;
}

void Application::listAllServer(std::map<std::string, std::vector<TcpServer::ptr> >& servers) {
    servers = m_servers;
}

}
