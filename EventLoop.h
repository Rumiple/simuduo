#pragma once

#include "noncopyable.h"
#include <functional>
#include <vector>
#include <atomic>
#include "Timestamp.h"
#include <memory>
#include <mutex>
#include "CurrentThread.h"


class Channel;
class Poller;


// 时间循环类，主要包含两个Channel和Poller类。Poller类中有ChannelMap，即fd和channel*
class EventLoop : noncopyable
{
public:
	using Functor = std::function<void()>;

	EventLoop();
	~EventLoop();

	void loop();
	void quit();
	Timestamp pollReturnTime() const { return pollReturnTime_; }

	void runInLoop(Functor cb); // 在当前loop中执行cb
	void queueInLoop(Functor cb); // 把cb放入队列中，唤醒loop所在的线程，执行cb

	void wakeup(); // 唤醒loop所在的线程

	// eventloop--->Poller
	void updateChannel(Channel* channel);
	void removeChannel(Channel* channel);
	bool hasChannel(Channel* channel);

	bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

private:
	void handleRead(); // 唤醒所需要的操作
	void doPendingFucntors(); // 执行回调

	using Channellist = std::vector<Channel*>;

	std::atomic_bool looping_; // 原子性操作，通过CAS（比较并交换）实现
	std::atomic_bool quit_; // 表示退出loop循环
	
	const pid_t threadId_; // 当前loop所在线程的id
	Timestamp pollReturnTime_; // poller返回发生事件的channels的时间点
	std::unique_ptr<Poller> poller_;
	int wakeupFd_; // 当mainLoop获取一个新用户的channel时，通过轮询算法选择一个subloop，通过wakeupFd_使用eventfd()来唤醒subloop。
	std::unique_ptr<Channel> wakeupChannel_;
	Channellist activeChannels_;

	std::atomic_bool callingPendingFunctors_; // 表示当前loop是否有需要执行的回调惭怍
	std::vector<Functor> pendingFunctors_; // 存储loop需要执行的所有回调操作
	std::mutex mutex_;

};