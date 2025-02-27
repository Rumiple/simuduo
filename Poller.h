#pragma once
/*
	����Poller��Ϊһ��������������������µĸ��������࣬��˿�������ʵ��IO���á�
*/

#include "noncopyable.h"
#include <vector>
#include <unordered_map>
#include "Timestamp.h"

class Channel;
class EventLoop;
class Poller
{
public:
	using ChannelList = std::vector<Channel*>;

	Poller(EventLoop* loop);
	virtual ~Poller() = default;

	// �����е�IO�����ṩͳһ�Ľӿڡ�
	virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0; 
	virtual void updateChannel(Channel* channel) = 0;
	virtual void removeChannel(Channel* channel) = 0;

	bool hasChannel(Channel* channel) const; // �ж�channel�Ƿ��ڵ�ǰ��Poller�С�
	static Poller* newDefaultPoller(EventLoop* loop); // ��ȡloopĬ�ϵ�Pollerʵ�֡���Ϊ������������ص���Poller�������࣬���Բ��������.cc�ļ��ж��塣
protected:
	using ChannelMap = std::unordered_map<int, Channel*>; // int��ʾsockfd��Channel*��sockfd������channelͨ������
	ChannelMap channels_;

private:
	EventLoop* ownerLoop_;



};