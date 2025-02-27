#pragma once
/*
	Channel是通道，封装了sockfd和其感兴趣的event，还绑定了poller监听返回的具体事件。
*/
#include "noncopyable.h"
#include <functional>
#include "Timestamp.h"
#include <memory>

class EventLoop;
class Timestamp;


class Channel : noncopyable
{
public:
	using EventCallback = std::function<void()>;
	using ReadEventCallback = std::function<void(Timestamp)>;

	Channel(EventLoop* loop, int fd);
	~Channel();

	void handleEvent(Timestamp receiveTime); // 调用对应的回调函数处理事件。
	// 接下来几个是设置回调函数对象。std::move()用于将一个左值强制转换成一个右值。
	void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); };
	void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); };
	void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); };
	void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); };
	
	void tie(const std::shared_ptr<void>&); // 防止当channel被手动remove掉后，channel还在执行回调操作。

	int fd() const { return fd_; }
	int events() const { return events_; }
	int set_revents(int revt) { revents_ = revt; }
	// 设置fd相应的事件状态。
	void enableReading() { events_ |= kReadEvent; update(); }
	void disableReading() { events_ &= ~kReadEvent; update(); }
	void enableWriting() { events_ |= kWriteEvent; update(); }
	void disableWriting() { events_ &= ~kWriteEvent; update(); }
	void disableAll() { events_ = kNoneEvent; update(); }
	// 返回fd当前的事件状态
	bool isNoneEvent() const { return events_ == kNoneEvent; }
	bool isWriting() const { return events_ & kWriteEvent; }
	bool isReading() const { return events_ & kReadEvent; }

	int index() { return index_; }
	int set_index(int idx) { index_ = idx; }

	EventLoop* ownerLoop() { return loop_; }
	void remove();

private:
	void update(); // 用epoll_ctl更改fd相应的事件。
	void handleEventWithGuard(Timestamp receiveTime);

	static const int kNoneEvent;
	static const int kReadEvent;
	static const int kWriteEvent;
	EventLoop* loop_;
	const int fd_; // poller监听的对象
	int events_; // 注册fd感兴趣的事件
	int revents_; // Poller返回的具体发生的事情
	int index_; // -1表示Channel未添加到poller中，1表示已添加，2表示从poller中删除。在Poller的派生类中用到。

	std::weak_ptr<void> tie_;
	bool tied_;

	// 各个具体事件的回调函数。因为Channel能够获取fd发生的具体事件revents
	ReadEventCallback readCallback_;
	EventCallback writeCallback_;
	EventCallback closeCallback_;
	EventCallback errorCallback_;

};
