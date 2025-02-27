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


// ʱ��ѭ���࣬��Ҫ��������Channel��Poller�ࡣPoller������ChannelMap����fd��channel*
class EventLoop : noncopyable
{
public:
	using Functor = std::function<void()>;

	EventLoop();
	~EventLoop();

	void loop();
	void quit();
	Timestamp pollReturnTime() const { return pollReturnTime_; }

	void runInLoop(Functor cb); // �ڵ�ǰloop��ִ��cb
	void queueInLoop(Functor cb); // ��cb��������У�����loop���ڵ��̣߳�ִ��cb

	void wakeup(); // ����loop���ڵ��߳�

	// eventloop--->Poller
	void updateChannel(Channel* channel);
	void removeChannel(Channel* channel);
	bool hasChannel(Channel* channel);

	bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

private:
	void handleRead(); // ��������Ҫ�Ĳ���
	void doPendingFucntors(); // ִ�лص�

	using Channellist = std::vector<Channel*>;

	std::atomic_bool looping_; // ԭ���Բ�����ͨ��CAS���Ƚϲ�������ʵ��
	std::atomic_bool quit_; // ��ʾ�˳�loopѭ��
	
	const pid_t threadId_; // ��ǰloop�����̵߳�id
	Timestamp pollReturnTime_; // poller���ط����¼���channels��ʱ���
	std::unique_ptr<Poller> poller_;
	int wakeupFd_; // ��mainLoop��ȡһ�����û���channelʱ��ͨ����ѯ�㷨ѡ��һ��subloop��ͨ��wakeupFd_ʹ��eventfd()������subloop��
	std::unique_ptr<Channel> wakeupChannel_;
	Channellist activeChannels_;

	std::atomic_bool callingPendingFunctors_; // ��ʾ��ǰloop�Ƿ�����Ҫִ�еĻص�����
	std::vector<Functor> pendingFunctors_; // �洢loop��Ҫִ�е����лص�����
	std::mutex mutex_;

};