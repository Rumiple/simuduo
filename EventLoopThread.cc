#include "EventLoopThread.h"
#include "EventLoop.h"



EventLoopThread::EventLoopThread(const ThreadInitCallback& cb, const std::string& name) 
	: loop_(nullptr)
	, exiting_(false)
	, thread_(std::bind(&EventLoopThread::threadFunc, this), name)
	, mutex_()
	, cond_()
	, callback_(cb) {}


EventLoopThread::~EventLoopThread() {
	exiting_ = true;
	if (loop_ != nullptr) {
		loop_->quit();
		thread_.join();
	}
}


EventLoop* EventLoopThread::startLoop() {
	//startLoop()创建一个新线程，然后等待新线程创建一个自己的EventLoop，
	// 这里通过condition_variable，当新EventLoop创建后，startLoop()通过条件变量返回loop的地址。
	thread_.start(); // 启动底层的新线程

	EventLoop* loop = nullptr;
	{
		std::unique_lock < std::mutex> lock(mutex_);
		while (loop_ == nullptr) {
			cond_.wait(lock);
		}
		loop = loop_; // 
	}
	return loop;
}


void EventLoopThread::threadFunc() { // 在新的线程中运行
	EventLoop loop; // 为新线程创建一个eventloop

	if (callback_) {
		callback_(&loop);
	}

	{
		std::unique_lock<std::mutex> lock(mutex_);
		loop_ = &loop;
		cond_.notify_one();
	}
	loop.loop(); // EventLoop.loop -----> Poller.poll()
}