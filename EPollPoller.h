#pragma once

/*
	һ��Eventloopֻ��һ��Poller���������ж��Channel��
	epoll��ʹ�ã�epoll_create, epoll_ctl, epoll_wait
	int epoll_ctl(int epfd, int op, int fd, struct epoll_event* event);eventָ���¼���
	int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout)��
	struct epoll_event {
		__uint32_t events; epoll�¼�
		epoll_data_t data; �û�����
	};
	typedef union epoll_data {
		void* ptr; ָ����fd��ص��û�����
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
	void update(int operation, Channel* channel); // ����channelͨ��

	using EventList = std::vector<epoll_event>;
	int epollfd_;
	EventList events_;


};