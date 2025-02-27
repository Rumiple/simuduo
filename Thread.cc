#include "Thread.h"
#include "CurrentThread.h"

#include <semaphore.h> // 因为在某些linux版本中需要显式链接pthread库，因此需要在项目的属性--编译器---所有选项---库依赖项中添加pthread


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
		thread_->detach(); // thread类提供的设置分离线程的方法
	}
}


void Thread::start() { // 一个thread对象就是一个新线程
	started_ = true;
	sem_t sem;
	sem_init(&sem, false, 0); // 第二个参数表示是否多线程，第三个参数表示是否赋值


	thread_ = std::shared_ptr<std::thread>(new std::thread([&]() { // 新建一个子线程
		tid_ = CurrentThread::tid(); // 获取线程的子进程
		sem_post(&sem);
		func_(); // 让子线程专门执行该函数
		}));

	sem_wait(&sem); // 用sem_wait等待获取上面新创建的线程的tid值

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