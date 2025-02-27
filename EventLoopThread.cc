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
	//startLoop()����һ�����̣߳�Ȼ��ȴ����̴߳���һ���Լ���EventLoop��
	// ����ͨ��condition_variable������EventLoop������startLoop()ͨ��������������loop�ĵ�ַ��
	thread_.start(); // �����ײ�����߳�

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


void EventLoopThread::threadFunc() { // ���µ��߳�������
	EventLoop loop; // Ϊ���̴߳���һ��eventloop

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