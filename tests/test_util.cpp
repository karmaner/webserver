#include "src/webserver.h"
#include <assert.h>

webserver::Logger::ptr g_logger = LOG_ROOT();

void test_assert() {
    LOG_INFO(g_logger) << webserver::BacktraceToString(10);
    WEBSERVER_ASSERT2(0 == 1, "abcdef xx");
}

int main(int argc, char* argv[]) {
    test_assert();
    std::cout << "This is the world end" << std::endl;
    return 0;
}