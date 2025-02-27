#include "EPollPoller.h"
#include "Logger.h"
#include "Channel.h"
#include <errno.h>
#include <cstring>
#include <unistd.h>


const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

EPollPoller::EPollPoller(EventLoop* loop)
	: Poller(loop)
	, epollfd_(::epoll_create1(EPOLL_CLOEXEC)) // epoll_create1相比于epoll_create不用输入size，并且可以额外指定EPOLL_CLOEXEC，当fork子进程后，子进程不会继承epoll实例的fd。
	, events_(kInitEventListSize) // vector<epoll_event>
{
	if (epollfd_ < 0) {
		LOG_FATAL("epoll_create error:%d \n", errno);
	}
}


EPollPoller::~EPollPoller() {
	::close(epollfd_);
}



Timestamp EPollPoller::poll(int timeoutMs, ChannelList * activeChannels) { // 通过epll_wait把activeChannels里发生事件的fd告知给eventloop。
	LOG_INFO("func=%s => fd total count:%lu \n", __FUNCTION__, channels_.size());
	int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs); // 第3个参数是把size_t安全的强制转换为int。&*events_.begin()的写法是先events_.begin()再*再&。
	int saveErrno = errno;
	Timestamp now(Timestamp::now());
	if (numEvents > 0) {
		LOG_INFO("%d events happened \n", numEvents);
		fillActiveChannels(numEvents, activeChannels);
		if (numEvents == events_.size()) {
			events_.resize(events_.size() * 2);
		}
	} 
	else if (numEvents == 0) { // 超时
		LOG_DEBUG("%s timeout! \n", __FUNCTION__);
	}
	else { // 有错误发生
		if (saveErrno != EINTR) {
			errno = saveErrno;
			LOG_ERROR("EPollPoller::Poll() err!");
		}
	}
	return now;

}

// Channel::update/remove---->EventLoop::updateChannel/removeChannel---->Poller::updateChannel/removeChannel
void EPollPoller::updateChannel(Channel * channel) {
	const int index = channel->index();
	LOG_INFO("func=%s => fd=%d events=%d index=%d \n", __FUNCTION__, channel->fd(), channel->events(), index);
	if (index == kNew || index == kDeleted) {
		if (index == kNew) {
			int fd = channel->fd();
			channels_[fd] = channel;
		}
		channel->set_index(kAdded);
		update(EPOLL_CTL_ADD, channel);
	}
	else {
		int fd = channel->fd();
		if (channel->isNoneEvent()) {
			update(EPOLL_CTL_DEL, channel);
			channel->set_index(kDeleted);
		}
		else {
			update(EPOLL_CTL_MOD, channel);
		}
	}
	
}


void EPollPoller::removeChannel(Channel* channel) { // 调用update来处理。
	int fd = channel->fd();
	int index = channel->index();
	channels_.erase(fd);

	LOG_INFO("func=%s => fd=%d \n", __FUNCTION__, fd);

	if (index == kAdded) {
		update(EPOLL_CTL_DEL, channel);
	}
	channel->set_index(kNew);
}


void EPollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const {
	for (int i = 0; i < numEvents; ++i) {
		Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
		channel->set_revents(events_[i].events);
		activeChannels->push_back(channel);
	}
}


void EPollPoller::update(int operation, Channel* channel) { // EPOLL中的add、del、mod的具体操作
	epoll_event event;
	memset(&event, 0, sizeof event);
	int fd = channel->fd();
	event.events = channel->events();
	event.data.fd = fd;
	event.data.ptr = channel;
	
	if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
		if (operation == EPOLL_CTL_DEL) {
			LOG_ERROR("epoll_ctl del error:%d\n", errno);
		}
		else {
			LOG_FATAL("epoll_ctl add/mod error:%d\n", errno);
		}
	}
}