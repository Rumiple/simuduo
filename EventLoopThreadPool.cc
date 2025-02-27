#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"

#include <memory>


EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg) 
	: baseLoop_(baseLoop)
	, name_(nameArg)
	, started_(false)
	, numThreads_(0)
	, next_(0) {}


EventLoopThreadPool::~EventLoopThreadPool() {}


void EventLoopThreadPool::start(const ThreadInitCallback& cb) {
	started_ = true;
	for (int i = 0; i < numThreads_; ++i) {
		char buf[name_.size() + 32];
		snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
		EventLoopThread* t = new EventLoopThread(cb, buf);
		threads_.push_back(std::unique_ptr<EventLoopThread>(t));
		loops_.push_back(t->startLoop()); // t新建一个线程和对应的loop，并返回其loop地址
	}

	if (numThreads_ == 0 && cb) { // 如果只有一个线程也就是baseloop对应的线程，并且还有事件，就让baseloop干活。
		cb(baseLoop_);
	}
}


EventLoop* EventLoopThreadPool::getNextLoop() { // 如果是在多线程，baseLoop_默认以轮询的方式分配channel给subloop
	EventLoop* loop = baseLoop_;
	if (!loops_.empty()) {
		loop = loops_[next_];
		++next_;
		if (next_ >= loops_.size()) {
			next_ = 0;
		}
	}
}


std::vector<EventLoop*> EventLoopThreadPool::getAllLoops() {
	if (loops_.empty()) {
		return std::vector<EventLoop*>(1, baseLoop_);
	}
	else {
		return loops_;
	}
}