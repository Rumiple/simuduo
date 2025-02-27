#pragma once
/*
	Channel��ͨ������װ��sockfd�������Ȥ��event��������poller�������صľ����¼���
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

	void handleEvent(Timestamp receiveTime); // ���ö�Ӧ�Ļص����������¼���
	// ���������������ûص���������std::move()���ڽ�һ����ֵǿ��ת����һ����ֵ��
	void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); };
	void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); };
	void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); };
	void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); };
	
	void tie(const std::shared_ptr<void>&); // ��ֹ��channel���ֶ�remove����channel����ִ�лص�������

	int fd() const { return fd_; }
	int events() const { return events_; }
	int set_revents(int revt) { revents_ = revt; }
	// ����fd��Ӧ���¼�״̬��
	void enableReading() { events_ |= kReadEvent; update(); }
	void disableReading() { events_ &= ~kReadEvent; update(); }
	void enableWriting() { events_ |= kWriteEvent; update(); }
	void disableWriting() { events_ &= ~kWriteEvent; update(); }
	void disableAll() { events_ = kNoneEvent; update(); }
	// ����fd��ǰ���¼�״̬
	bool isNoneEvent() const { return events_ == kNoneEvent; }
	bool isWriting() const { return events_ & kWriteEvent; }
	bool isReading() const { return events_ & kReadEvent; }

	int index() { return index_; }
	int set_index(int idx) { index_ = idx; }

	EventLoop* ownerLoop() { return loop_; }
	void remove();

private:
	void update(); // ��epoll_ctl����fd��Ӧ���¼���
	void handleEventWithGuard(Timestamp receiveTime);

	static const int kNoneEvent;
	static const int kReadEvent;
	static const int kWriteEvent;
	EventLoop* loop_;
	const int fd_; // poller�����Ķ���
	int events_; // ע��fd����Ȥ���¼�
	int revents_; // Poller���صľ��巢��������
	int index_; // -1��ʾChannelδ��ӵ�poller�У�1��ʾ����ӣ�2��ʾ��poller��ɾ������Poller�����������õ���

	std::weak_ptr<void> tie_;
	bool tied_;

	// ���������¼��Ļص���������ΪChannel�ܹ���ȡfd�����ľ����¼�revents
	ReadEventCallback readCallback_;
	EventCallback writeCallback_;
	EventCallback closeCallback_;
	EventCallback errorCallback_;

};
