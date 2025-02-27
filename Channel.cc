#include "Channel.h"
#include <sys/epoll.h>
#include "EventLoop.h"
#include "Logger.h"

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd)
	: loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false)
{}

Channel::~Channel() {}

void Channel::tie(const std::shared_ptr<void>& obj) { // 新连接一个TcpConnection对象的时候调用，为了延长TcpConnection对象的生存期。
	tie_ = obj;
	tied_ = true;
}

void Channel::update() {
	// 通过channel所属的EventLoop，调用poller的相应方法，注册fd的events事件。
	loop_->updateChannel(this);

}void Channel::remove() {
	// 在channel所属的EventLoop中，删除当前的channel。
	loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime) {
	if (tied_) {
		std::shared_ptr<void> guard = tie_.lock();
		if (guard) {
			handleEventWithGuard(receiveTime);
		}
	}
	else {
		handleEventWithGuard(receiveTime);
	}
}

void Channel::handleEventWithGuard(Timestamp receiveTime) { // 根据fd的事件使用回调函数
	LOG_INFO("channel handleEvent revents:%d", revents_);
	if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) { // EPOLLHUP 表示读写都关闭
		if (closeCallback_) {
			closeCallback_();
		}
	}

	if (revents_ & EPOLLERR) { // EPOLLERR表示有错误。
		if (errorCallback_) {
			errorCallback_();
		}
	}

	if (revents_ & (EPOLLIN | EPOLLPRI)) { // EPOLLIN表示fd可读，EPOLLPRI 表示有紧急数据
		if (readCallback_) {
			readCallback_(receiveTime);
		}
	}

	if (revents_ & EPOLLOUT) {// EPOLLOUT表示可写
		if (writeCallback_) {
			writeCallback_();
		}
	}
}