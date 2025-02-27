#pragma once

#include "noncopyable.h"


#include <functional>
#include <string>
#include <vector>
#include <memory>


class EventLoop;
class EventLoopThread;


class EventLoopThreadPool : noncopyable
{
public:
	using ThreadInitCallback = std::function<void(EventLoop*)>;

	EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg);
	~EventLoopThreadPool();

	void setThreadNum(int numThreads) { numThreads_ = numThreads; }
	void start(const ThreadInitCallback& cb = ThreadInitCallback());
	

	EventLoop* getNextLoop();

	std::vector<EventLoop*> getAllLoops();

	bool started() const { return started_; }
	const std::string name() const { return name_; }

private:
	EventLoop* baseLoop_; // 用户一开始创建的loop。
	bool started_;
	std::string name_;
	int numThreads_;
	int next_; // 轮询的下标
	std::vector<std::unique_ptr<EventLoopThread>> threads_; // 存储的是创建的所有的线程
	std::vector<EventLoop*> loops_; // 线程绑定的loop都是栈上的对象，因此不需要手动删除




};




