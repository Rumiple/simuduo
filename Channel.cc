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

void Channel::tie(const std::shared_ptr<void>& obj) { // ������һ��TcpConnection�����ʱ����ã�Ϊ���ӳ�TcpConnection����������ڡ�
	tie_ = obj;
	tied_ = true;
}

void Channel::update() {
	// ͨ��channel������EventLoop������poller����Ӧ������ע��fd��events�¼���
	loop_->updateChannel(this);

}void Channel::remove() {
	// ��channel������EventLoop�У�ɾ����ǰ��channel��
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

void Channel::handleEventWithGuard(Timestamp receiveTime) { // ����fd���¼�ʹ�ûص�����
	LOG_INFO("channel handleEvent revents:%d", revents_);
	if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) { // EPOLLHUP ��ʾ��д���ر�
		if (closeCallback_) {
			closeCallback_();
		}
	}

	if (revents_ & EPOLLERR) { // EPOLLERR��ʾ�д���
		if (errorCallback_) {
			errorCallback_();
		}
	}

	if (revents_ & (EPOLLIN | EPOLLPRI)) { // EPOLLIN��ʾfd�ɶ���EPOLLPRI ��ʾ�н�������
		if (readCallback_) {
			readCallback_(receiveTime);
		}
	}

	if (revents_ & EPOLLOUT) {// EPOLLOUT��ʾ��д
		if (writeCallback_) {
			writeCallback_();
		}
	}
}