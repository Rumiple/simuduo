#include "Thread.h"
#include "CurrentThread.h"

#include <semaphore.h> // ��Ϊ��ĳЩlinux�汾����Ҫ��ʽ����pthread�⣬�����Ҫ����Ŀ������--������---����ѡ��---�������������pthread


std::atomic_int Thread::numCreated_(0);


Thread::Thread(ThreadFunc func, const std::string& name)
	: started_(false)
	, joined_(false)
	, tid_(0)
	, func_(std::move(func))
	, name_(name)
{
	setDefaultName();
}


Thread::~Thread() {
	if (started_ && !joined_) {
		thread_->detach(); // thread���ṩ�����÷����̵߳ķ���
	}
}


void Thread::start() { // һ��thread�������һ�����߳�
	started_ = true;
	sem_t sem;
	sem_init(&sem, false, 0); // �ڶ���������ʾ�Ƿ���̣߳�������������ʾ�Ƿ�ֵ


	thread_ = std::shared_ptr<std::thread>(new std::thread([&]() { // �½�һ�����߳�
		tid_ = CurrentThread::tid(); // ��ȡ�̵߳��ӽ���
		sem_post(&sem);
		func_(); // �����߳�ר��ִ�иú���
		}));

	sem_wait(&sem); // ��sem_wait�ȴ���ȡ�����´������̵߳�tidֵ

}


void Thread::join() {
	joined_ = true;
	thread_->join();
}


void Thread::setDefaultName() {
	int num = ++numCreated_;
	if (name_.empty()) {
		char buf[32] = { 0 };
		snprintf(buf, sizeof buf, "Thread%d", num);
		name_ = buf;
	}
}