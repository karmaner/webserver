#include "my_module.h"
#include "src/basic/config.h"
#include "src/basic/log.h"

namespace name_space {

static webserver::Logger::ptr g_logger = LOG_ROOT();

MyModule::MyModule()
    :webserver::Module("project_name", "1.0", "") {
}

bool MyModule::onLoad() {
    LOG_INFO(g_logger) << "onLoad";
    return true;
}

bool MyModule::onUnload() {
    LOG_INFO(g_logger) << "onUnload";
    return true;
}

bool MyModule::onServerReady() {
    LOG_INFO(g_logger) << "onServerReady";
    return true;
}

bool MyModule::onServerUp() {
    LOG_INFO(g_logger) << "onServerUp";
    return true;
}

}

extern "C" {

webserver::Module* CreateModule() {
    webserver::Module* module = new name_space::MyModule;
    LOG_INFO(name_space::g_logger) << "CreateModule " << module;
    return module;
}

void DestoryModule(webserver::Module* module) {
    LOG_INFO(name_space::g_logger) << "CreateModule " << module;
    delete module;
}

}
