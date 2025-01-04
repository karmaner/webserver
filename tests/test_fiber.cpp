#include "src/webserver.h"

webserver::Logger::ptr g_logger = LOG_ROOT();

void run_in_fiber() {
	LOG_INFO(g_logger) << "run_in_fiber begin";
	webserver::Fiber::YieldToHold();
	LOG_INFO(g_logger) << "run_in_fiber end";
	webserver::Fiber::YieldToHold();
}

void test_fiber() {
	LOG_INFO(g_logger) << "main begin -1";
	{
		webserver::Fiber::GetThis();
		LOG_INFO(g_logger) << "main begin";
		webserver::Fiber::ptr fiber(new webserver::Fiber(run_in_fiber));
		fiber->swapIn();
		LOG_INFO(g_logger) << "main after swapIpn";
		fiber->swapIn();
		LOG_INFO(g_logger) << "main after end";
		fiber->swapIn();
	}
	LOG_INFO(g_logger) << "main after end2";
}

int main(int argc, char* argv[]) {
	webserver::Thread::SetName("main");

	std::vector<webserver::Thread::ptr> thrs;
	for(int i = 0; i < 3; ++i) {
		thrs.push_back(webserver::Thread::ptr(
						new webserver::Thread(&test_fiber, "name_" + std::to_string(i))));
	}
	for(auto i : thrs) {
		i->join();
	}
	return 0;
}
