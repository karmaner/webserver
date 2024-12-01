#include "src/basic/module.h"
#include "src/basic/singleton.h"
#include "src/basic/log.h"
#include <iostream>

static webserver::Logger::ptr g_logger = LOG_ROOT();

class A {
public:
    A() {
        std::cout << "A::A " << this << std::endl;
    }

    ~A() {
        std::cout << "A::~A " << this << std::endl;
    }

};

class MyModule : public webserver::RockModule {
public:
    MyModule()
        :RockModule("hello", "1.0", "") {
        //webserver::Singleton<A>::GetInstance();
    }

    bool onLoad() override {
        webserver::Singleton<A>::GetInstance();
        std::cout << "-----------onLoad------------" << std::endl;
        return true;
    }

    bool onUnload() override {
        webserver::Singleton<A>::GetInstance();
        std::cout << "-----------onUnload------------" << std::endl;
        return true;
    }

    bool onServerReady() {
        registerService("rock", "sylar.top", "blog");
        return true;
    }

    bool handleRockRequest(webserver::RockRequest::ptr request
                        ,webserver::RockResponse::ptr response
                        ,webserver::RockStream::ptr stream) {
        LOG_INFO(g_logger) << "handleRockRequest " << request->toString();
        response->setResult(0);
        response->setResultStr("ok");
        response->setBody("echo: " + request->getBody());
        return true;
    }

    bool handleRockNotify(webserver::RockNotify::ptr notify 
                        ,webserver::RockStream::ptr stream) {
        LOG_INFO(g_logger) << "handleRockNotify " << notify->toString();
        return true;
    }
};

extern "C" {

webserver::Module* CreateModule() {
    webserver::Singleton<A>::GetInstance();
    std::cout << "=============CreateModule=================" << std::endl;
    return new MyModule;
}

void DestoryModule(webserver::Module* ptr) {
    std::cout << "=============DestoryModule=================" << std::endl;
    delete ptr;
}

}
