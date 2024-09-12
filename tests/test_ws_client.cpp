#include "src/http/ws_connection.h"
#include "src/basic/iomanager.h"
#include "src/basic/util.h"

void run() {
    auto rt = webserver::http::WSConnection::Create("http://127.0.0.1:8020/webserver", 1000);
    if(!rt.second) {
        std::cout << rt.first->toString() << std::endl;
        return;
    }

    auto conn = rt.second;
    while(true) {
        //for(int i = 0; i < 1100; ++i) {
        for(int i = 0; i < 1; ++i) {
            conn->sendMessage(webserver::random_string(60), webserver::http::WSFrameHead::TEXT_FRAME, false);
        }
        conn->sendMessage(webserver::random_string(65), webserver::http::WSFrameHead::TEXT_FRAME, true);
        auto msg = conn->recvMessage();
        if(!msg) {
            break;
        }
        std::cout << "opcode=" << msg->getOpcode()
                    << " data=" << msg->getData() << std::endl;

        sleep(10);
    }
}

int main(int argc, char* argv[]) {
    srand(time(0));
    webserver::IOManager iom(1);
    iom.schedule(run);
    return 0;
}