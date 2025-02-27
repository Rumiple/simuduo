#include "Poller.h"
#include <stdlib.h> // ������ļ�����Ҫ���õ��˻�ȡ���������ĺ�����
#include "EPollPoller.h"


Poller* Poller::newDefaultPoller(EventLoop* loop) {
	if (::getenv("MUDUO_USE_POLL")) {
		return nullptr;
	}
	else {
		return new EPollPoller(loop);
	}
}