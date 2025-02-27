#pragma once

/*
	一个Eventloop只有一个Poller，但可以有多个Channel。
	epoll的使用：epoll_create, epoll_ctl, epoll_wait
	int epoll_ctl(int epfd, int op, int fd, struct epoll_event* event);event指定事件。
	int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout)。
	struct epoll_event {
		__uint32_t events; epoll事件
		epoll_data_t data; 用户数据
	};
	typedef union epoll_data {
		void* ptr; 指定与fd相关的用户数据
		int fd;
		uint32_t u32;
		uint64_t u64;
	}epoll_data_t;
*/

#include "Poller.h"
#include "Timestamp.h"
#include <vector>
#include <sys/epoll.h>

class Channel;

class EPollPoller : public Poller
{
public:
	EPollPoller(EventLoop* loop);
	~EPollPoller() override;

	Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
	void updateChannel(Channel* channel) override;
	void removeChannel(Channel* channel) override;


private:
	static const int kInitEventListSize = 16;

	void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
	void update(int operation, Channel* channel); // 更新channel通道

	using EventList = std::vector<epoll_event>;
	int epollfd_;
	EventList events_;


};