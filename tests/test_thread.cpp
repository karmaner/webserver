#include "src/webserver.h"
#include <unistd.h>

webserver::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();

int count = 0;
// webserver::RWMutex s_mutex;
webserver::Mutex s_mutex;

void func1()
{
    WEBSERVER_LOG_INFO(g_logger) << "name: " << webserver::Thread::GetName()
                                    << " this.name: " << webserver::Thread::GetThis()->getName()
                                    << " id:" << webserver::GetThreadId()
                                    << " this.id:" << webserver::Thread::GetThis()->getId();
    for(int i = 0; i < 100000; ++i) {
        // webserver::RWMutex::WriteLock lock(s_mutex);
        webserver::Mutex::Lock lock(s_mutex);
        ++count;
    }
}

void func2()
{

    while (true) {
        WEBSERVER_LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx "
                                        << webserver::Thread::GetName() << " "
                                        << webserver::GetThreadId();
        webserver::Mutex::Lock lock(s_mutex);
    }
}

void func3() 
{
    
    while (true) {
        WEBSERVER_LOG_INFO(g_logger) << "==================================== "
                                        << webserver::Thread::GetName() << " "
                                        << webserver::GetThreadId();
        webserver::Mutex::Lock lock(s_mutex);
    }
}

int main(int argc, char *argv[])
{
    YAML::Node root = YAML::LoadFile("/home/karmaner/myRepo/webserver/bin/conf/log2.yml");
    webserver::Config::LoadFromYaml(root);

    std::vector<webserver::Thread::ptr> thrs;
    for (int i = 0; i < 1; ++i)
    {
        webserver::Thread::ptr thr(new webserver::Thread(&func2, "name_" + std::to_string(i * 2)));
        webserver::Thread::ptr thr2(new webserver::Thread(&func3, "name_" + std::to_string(i * 2 + 1)));
        thrs.push_back(thr);
        thrs.push_back(thr2);
    }

    for (size_t i = 0; i < thrs.size(); ++i)
    {
        thrs[i]->join();
    }
    WEBSERVER_LOG_INFO(g_logger) << "Thread test end";
    WEBSERVER_LOG_INFO(g_logger) << "count=" << count;
    return 0;
}