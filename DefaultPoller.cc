#include "Poller.h"
#include <stdlib.h> // 在这个文件中主要是用到了获取环境变量的函数。
#include "EPollPoller.h"


Poller* Poller::newDefaultPoller(EventLoop* loop) {
	if (::getenv("MUDUO_USE_POLL")) {
		return nullptr;
	}
	else {
		return new EPollPoller(loop);
	}
}