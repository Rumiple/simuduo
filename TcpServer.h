#pragma once

#include "noncopyable.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "EventLoopThreadPool.h"
#include "Callback.h"
#include "TcpConnection.h"
#include "Buffer.h"

#include <functional>
#include <memory>
#include <atomic>
#include <unordered_map>

// ����������
class TcpServer : noncopyable
{
public:
	using ThreadInitCallback = std::function<void(EventLoop*)>;

	enum Option // ��ʾ�Զ˿��Ƿ������
	{
		kNoReusePort,
		kReusePort
	};

	TcpServer(EventLoop* loop, 
		const InetAddress& listenAddr, 
		const std::string& nameArg,
		Option option = kNoReusePort);
	~TcpServer();

	void setThreadNum(int numThreads); // ���õײ�subloop�ĸ���
	void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }
	void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
	void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }

	void start(); // ��������������


private:
	void newConnection(int sockfd, const InetAddress& peeraddr);
	void removeConnection(const TcpConnectionPtr& conn);
	void removeConnectionInLoop(const TcpConnectionPtr& conn);


	using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

	EventLoop* loop_;
	const std::string ipPort_;
	const std::string name_;
	std::unique_ptr<Acceptor> acceptor_;
	std::shared_ptr<EventLoopThreadPool> threadPool_;

	ConnectionCallback connectionCallback_; // ��������ʱ�Ļص�
	MessageCallback messageCallback_; // �ж�д��Ϣʱ�Ļص�
	WriteCompleteCallback writeCompleteCallback_; // ��Ϣ��������Ժ�Ļص���

	ThreadInitCallback threadInitCallback_; // loop�̳߳�ʼ���Ļص�
	std::atomic_int started_;

	int nextConnId_;
	ConnectionMap connections_; // �������е�����


};