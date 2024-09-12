#include "src/basic/module.h"
#include "src/basic/singleton.h"
#include <iostream>

class A {
public:
    A() {
        std::cout << "A::A " << this << std::endl;
    }

    ~A() {
        std::cout << "A::~A " << this << std::endl;
    }

};

class MyModule : public webserver::Module {
public:
    MyModule()
        :Module("hello", "1.0", "") {
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
