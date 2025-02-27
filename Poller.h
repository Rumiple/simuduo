#pragma once
/*
	这里Poller作为一个抽象基类来调用它其下的各种派生类，因此可以用来实现IO复用。
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

	// 给所有的IO复用提供统一的接口。
	virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0; 
	virtual void updateChannel(Channel* channel) = 0;
	virtual void removeChannel(Channel* channel) = 0;

	bool hasChannel(Channel* channel) const; // 判断channel是否在当前的Poller中。
	static Poller* newDefaultPoller(EventLoop* loop); // 获取loop默认的Poller实现。因为在这个函数返回的是Poller的派生类，所以不在这里的.cc文件中定义。
protected:
	using ChannelMap = std::unordered_map<int, Channel*>; // int表示sockfd，Channel*是sockfd所属的channel通道类型
	ChannelMap channels_;

private:
	EventLoop* ownerLoop_;



};