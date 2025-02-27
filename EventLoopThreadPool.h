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
	EventLoop* baseLoop_; // �û�һ��ʼ������loop��
	bool started_;
	std::string name_;
	int numThreads_;
	int next_; // ��ѯ���±�
	std::vector<std::unique_ptr<EventLoopThread>> threads_; // �洢���Ǵ��������е��߳�
	std::vector<EventLoop*> loops_; // �̰߳󶨵�loop����ջ�ϵĶ�����˲���Ҫ�ֶ�ɾ��




};




