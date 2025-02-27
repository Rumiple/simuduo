#include "EventLoop.h"
#include "Logger.h"
#include "Poller.h"
#include "Channel.h"

#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <memory>

__thread EventLoop* t_loopInThisThread = 0; // 防止一个线程创建多个EventLoop
const int kPollTimeMs = 10000; // 定义了Poller IO复用接口默认的超时时间


int createEventfd() { // 创建wakeupfd，用来通知唤醒subReactor处理新来的channel
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
	
	wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));// 设置wakeupfd的事件类型和发生事件后的回调操作
	wakeupChannel_->enableReading(); // 每一个eventLoop都将监听wakeupChannel的EPOLLIN读事件了。

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
		// 以下监听两种fd，客户端的fd和需要唤醒的fd。
		pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_); // Poller把监听的活跃的事件填到activeChannels_里。
		for (Channel* channel : activeChannels_) {
			channel->handleEvent(pollReturnTime_); // 执行handleRead()操作来唤醒subReactor
		}
		doPendingFucntors(); // 执行之前mainloop注册的回调。
	}
	LOG_INFO("EventLoop %p stop looping. \n", this);
	looping_ = false;
}


// 退出quit有两种情况，在自己线程中quit；另一种情况是在其他线程中退出。
void EventLoop::quit() {
	quit_ = true;
	if (!isInLoopThread()) {
		wakeup();
	}
}


// 如果我们想让IO执行一定的任务，如果是在当前的线程中，就会马上执行cb()；如果是在其他线程中调用，因此会先将
// cb异步添加到队列中，当loop内处理完事件后，就会执行doPendingFunctors()。
void EventLoop::runInLoop(Functor cb) { // 在当前loop线程中执行回调函数
	if (isInLoopThread()) {
		cb();
	}
	else {
		queueInLoop(cb);
	}
}


void EventLoop::queueInLoop(Functor cb) { // 把cb放入队列，唤醒loop所在的线程，执行cb
	{
		std::unique_lock<std::mutex> lock(mutex_);
		pendingFunctors_.emplace_back(cb);
	}
	// 这里加上callingPendingFunctors_的意思是如果当前loop正在执行回调，执行完后，回到loop()里时，loop会被poll阻塞。
	// 因此需要通过wakeup()触发新的事件循环，避免新任务被延迟到下一次事件循环中。
	// 确保加入的新任务也能被及时处理（这个意思是不会等到poll等待超时或者有新事件发生才被执行）。
	if (!isInLoopThread() || callingPendingFunctors_) { 
		wakeup();
	}
}


void EventLoop::wakeup() { // 向wakeupfd_写数据来唤醒。
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
	for (const Functor& functor : functors) { // 执行当前loop需要执行的回调操作
		functor();
	}
}


void EventLoop::handleRead() { // 发8字节来唤醒subReactor。
	uint64_t one = 1;
	ssize_t n = read(wakeupFd_, &one, sizeof one);
	if (n != sizeof one) {
		LOG_ERROR("EventLoop::handleRead() reads %d bytes instead of 8", n);
	}
}


