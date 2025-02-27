#pragma once

#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"

#include <functional>

class InetAddress;
class EventLoop;

class Acceptor : noncopyable
{
public:
	using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;
	Acceptor(EventLoop* loop, const InetAddress& listen, bool reuseport);
	~Acceptor();

	void setNewConnectionCallback(const NewConnectionCallback& cb) {
		newConnectionCallback_ = std::move(cb);
	}

	bool listenning() const { return listenning_; }
	void listen();

private:
	void handleRead();

	EventLoop* loop_; // 用户定义的baseloop，也就是mainloop
	Socket acceptSocket_;
	Channel acceptChannel_;
	NewConnectionCallback newConnectionCallback_;
	bool listenning_;



};
