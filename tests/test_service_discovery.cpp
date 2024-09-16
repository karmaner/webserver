#include "src/streams/service_discovery.h"
#include "src/basic/iomanager.h"

void run() {
    webserver::ZKServiceDiscovery::ptr zksd(new webserver::ZKServiceDiscovery("127.0.0.1:21811"));
    zksd->registerServer("blog", "chat", "127.0.0.1:1111", "abcd");
    zksd->registerServer("blog", "chat2", "127.0.0.1:1111", "abcd");
    zksd->queryServer("blog", "chat");
    zksd->setSelfInfo("127.0.0.1:2222");
    zksd->setSelfData("aaaa");

    zksd->start();
}

int main(int argc, char** argv) {
    webserver::IOManager iom(1);
    iom.addTimer(1000, [](){}, true);
    iom.schedule(run);
    return 0;
}