#include "src/webserver.h"

static webserver::Logger::ptr g_logger = LOG_ROOT();

void test_fiber() {
	static int s_count = 5;
	LOG_INFO(g_logger) << "test in fiber s_count=" << s_count;

	sleep(1);
	if(--s_count >= 0) {
		webserver::Scheduler::GetThis()->schedule(&test_fiber, webserver::GetThreadId());
	}
}

int main(int argc, char* argv[]) {
	LOG_INFO(g_logger) << "main";
	webserver::Scheduler sc(3, true, "test");
	sc.start();
	sleep(2);
	LOG_INFO(g_logger) << "schedule";
	sc.schedule(&test_fiber);
	sc.stop();
	LOG_INFO(g_logger) << "over";
	return 0;
}
