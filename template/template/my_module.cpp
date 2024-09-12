#include "my_module.h"
#include "src/basic/config.h"
#include "src/basic/log.h"

namespace name_space {

static webserver::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();

MyModule::MyModule()
    :webserver::Module("project_name", "1.0", "") {
}

bool MyModule::onLoad() {
    WEBSERVER_LOG_INFO(g_logger) << "onLoad";
    return true;
}

bool MyModule::onUnload() {
    WEBSERVER_LOG_INFO(g_logger) << "onUnload";
    return true;
}

bool MyModule::onServerReady() {
    WEBSERVER_LOG_INFO(g_logger) << "onServerReady";
    return true;
}

bool MyModule::onServerUp() {
    WEBSERVER_LOG_INFO(g_logger) << "onServerUp";
    return true;
}

}

extern "C" {

webserver::Module* CreateModule() {
    webserver::Module* module = new name_space::MyModule;
    WEBSERVER_LOG_INFO(name_space::g_logger) << "CreateModule " << module;
    return module;
}

void DestoryModule(webserver::Module* module) {
    WEBSERVER_LOG_INFO(name_space::g_logger) << "CreateModule " << module;
    delete module;
}

}
