#pragma once

#include "noncopyable.h"
#include "InetAddress.h"
#include "Callback.h"
#include "Buffer.h"
#include "Timestamp.h"

#include <memory>
#include <string>
#include <atomic>
class EventLoop;
class Channel;
class Socket;

/*
* TcpServerͨ��Acceptor����һ�����û����ӣ�ͨ��accept�����õ�connfd��
* Ȼ��TcpConnection���ûص���Channel����Poller��⵽�¼������������ö�Ӧ�Ļص���
*/
// ����ɹ�����ͨ����·�ĸ��ֲ�����
class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
	TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
		const InetAddress& localAddr, const InetAddress& peerAddr);
	~TcpConnection();

	EventLoop* getLoop() const { return loop_; }
	const std::string& name() const { return name_; }
	const InetAddress& localAddress() const { return localAddr_; }
	const InetAddress& peerAddress() const { return peerAddr_; }

	bool connected() const { return state_ == kConnected; }


	void setConnectionCallback(const ConnectionCallback& cb) {
		connectionCallback_ = cb;
	}
	void setMessageCallback(const MessageCallback& cb) {
		messageCallback_ = cb;
	}
	void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
		writeCompleteCallback_ = cb;
	}
	void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark) {
		highWaterMarkCallback_ = cb;
		highWaterMark_ = highWaterMark;

	}
	void setCloseCallback(const CloseCallback& cb) {
		closeCallback_ = cb;
	}

	void connectEstablished();
	void connectDestroyed();
	void send(const std::string& buf);
	void shutdown(); // �رյ�ǰ����

private:
	enum StateE {kDisconnected, kConnecting, kConnected, kDisconnecting};
	void setState(StateE state) { state_ = state; }

	void handleRead(Timestamp receiveTime);
	void handleWrite();
	void handleClose();
	void handleError();

	

	void sendInLoop(const void* data, size_t len);
	void shutdownInLoop();
	


	EventLoop* loop_;
	const std::string name_;
	std::atomic_int state_;
	bool reading_;

	std::unique_ptr<Socket> socket_;
	std::unique_ptr<Channel> channel_;

	const InetAddress localAddr_;
	const InetAddress peerAddr_;

	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	WriteCompleteCallback writeCompleteCallback_;
	CloseCallback closeCallback_;
	HighWaterMarkCallback highWaterMarkCallback_;

	size_t highWaterMark_;

	Buffer inputBuffer_;
	Buffer outputBuffer_;
};