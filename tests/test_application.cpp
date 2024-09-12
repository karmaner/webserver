#include "src/application/application.h"

int main(int argc, char* argv[]) {
    webserver::Application app;
    if(app.init(argc, argv)) {
        return app.run();
    }
    return 0;
}