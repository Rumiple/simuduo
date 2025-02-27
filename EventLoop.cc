#include "EventLoop.h"
#include "Logger.h"
#include "Poller.h"
#include "Channel.h"

#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <memory>

__thread EventLoop* t_loopInThisThread = 0; // ��ֹһ���̴߳������EventLoop
const int kPollTimeMs = 10000; // ������Poller IO���ýӿ�Ĭ�ϵĳ�ʱʱ��


int createEventfd() { // ����wakeupfd������֪ͨ����subReactor����������channel
	int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (evtfd < 0) {
		LOG_FATAL("eventfd error:%d \n", errno);
		abort();
	}
	return evtfd;
}


EventLoop::EventLoop()
	: looping_(false)
	, quit_(false)
	, callingPendingFunctors_(false)
	, threadId_(CurrentThread::tid())
	, poller_(Poller::newDefaultPoller(this))
	, wakeupFd_(createEventfd())
	, wakeupChannel_(new Channel(this, wakeupFd_))
{
	LOG_DEBUG("EventLoop created %p in thread %d \n", this, threadId_);
	if (t_loopInThisThread) {
		LOG_FATAL("Another EventLoop %p exists in this thread %d \n", t_loopInThisThread, threadId_);
	}
	else {
		t_loopInThisThread = this;
	}
	
	wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));// ����wakeupfd���¼����ͺͷ����¼���Ļص�����
	wakeupChannel_->enableReading(); // ÿһ��eventLoop��������wakeupChannel��EPOLLIN���¼��ˡ�

}


EventLoop::~EventLoop() {
	wakeupChannel_->disableAll();
	wakeupChannel_->remove();
	::close(wakeupFd_);
	t_loopInThisThread = nullptr;
}


void EventLoop::loop() {
	looping_ = true;
	quit_ = false;
	LOG_INFO("EventLoop %p start looping \n", this);

	while (!quit_) {
		activeChannels_.clear();
		// ���¼�������fd���ͻ��˵�fd����Ҫ���ѵ�fd��
		pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_); // Poller�Ѽ����Ļ�Ծ���¼��activeChannels_�
		for (Channel* channel : activeChannels_) {
			channel->handleEvent(pollReturnTime_); // ִ��handleRead()����������subReactor
		}
		doPendingFucntors(); // ִ��֮ǰmainloopע��Ļص���
	}
	LOG_INFO("EventLoop %p stop looping. \n", this);
	looping_ = false;
}


// �˳�quit��������������Լ��߳���quit����һ��������������߳����˳���
void EventLoop::quit() {
	quit_ = true;
	if (!isInLoopThread()) {
		wakeup();
	}
}


// �����������IOִ��һ��������������ڵ�ǰ���߳��У��ͻ�����ִ��cb()��������������߳��е��ã���˻��Ƚ�
// cb�첽��ӵ������У���loop�ڴ������¼��󣬾ͻ�ִ��doPendingFunctors()��
void EventLoop::runInLoop(Functor cb) { // �ڵ�ǰloop�߳���ִ�лص�����
	if (isInLoopThread()) {
		cb();
	}
	else {
		queueInLoop(cb);
	}
}


void EventLoop::queueInLoop(Functor cb) { // ��cb������У�����loop���ڵ��̣߳�ִ��cb
	{
		std::unique_lock<std::mutex> lock(mutex_);
		pendingFunctors_.emplace_back(cb);
	}
	// �������callingPendingFunctors_����˼�������ǰloop����ִ�лص���ִ����󣬻ص�loop()��ʱ��loop�ᱻpoll������
	// �����Ҫͨ��wakeup()�����µ��¼�ѭ���������������ӳٵ���һ���¼�ѭ���С�
	// ȷ�������������Ҳ�ܱ���ʱ���������˼�ǲ���ȵ�poll�ȴ���ʱ���������¼������ű�ִ�У���
	if (!isInLoopThread() || callingPendingFunctors_) { 
		wakeup();
	}
}


void EventLoop::wakeup() { // ��wakeupfd_д���������ѡ�
	uint64_t one = 1;
	ssize_t n = write(wakeupFd_, &one, sizeof one);
	if (n != sizeof one) {
		LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8 \n", n);
	}
}


void EventLoop::updateChannel(Channel* channel) {
	poller_->updateChannel(channel);
}


void EventLoop::removeChannel(Channel* channel) {
	poller_->removeChannel(channel);
}


bool EventLoop::hasChannel(Channel* channel) {
	return poller_->hasChannel(channel);
}


void EventLoop::doPendingFucntors() {
	std::vector<Functor> functors; 
	callingPendingFunctors_ = true;
	{
		std::unique_lock<std::mutex> lock(mutex_);
		functors.swap(pendingFunctors_);
	}
	for (const Functor& functor : functors) { // ִ�е�ǰloop��Ҫִ�еĻص�����
		functor();
	}
}


void EventLoop::handleRead() { // ��8�ֽ�������subReactor��
	uint64_t one = 1;
	ssize_t n = read(wakeupFd_, &one, sizeof one);
	if (n != sizeof one) {
		LOG_ERROR("EventLoop::handleRead() reads %d bytes instead of 8", n);
	}
}


