#include "TcpServer.h"
#include "Logger.h"
#include "TcpConnection.h"

#include <functional>
#include <strings.h>




static EventLoop* CheckLoopNotNull(EventLoop* loop) {
	if (loop == nullptr) {
		LOG_FATAL("%s:%s:%d mainLoop is null! \n", __FILE__, __FUNCTION__, __LINE__);
	}
	return loop;
}

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& nameArg, Option option)
	: loop_(loop)
	, ipPort_(listenAddr.toIpPort())
	, name_(nameArg)
	, acceptor_(new Acceptor(loop, listenAddr, option == kReusePort))
	, threadPool_(new EventLoopThreadPool(loop, name_))
	, connectionCallback_()
	, messageCallback_()
	, nextConnId_(1)
{
	acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this,
		std::placeholders::_1, std::placeholders::_2)); // st::placeholders是函数用的占位符,两个参数对应的部分是sockfd和peerAddr

}


TcpServer::~TcpServer() {
	for (auto& item : connections_) {
		TcpConnectionPtr conn(item.second);
		item.second.reset(); // TcpConnectionPtr这个强智能指针不会再指向TcpConnection对象，即可以自动释放对应的TcpConnection对象的资源。
		conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
	}
}


void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
	// 根据轮询算法选择一个subloop，然后通过runInLoop直接执行任务或者唤醒subloop，
	// 把当前的connfd封装成channel分发给subloop
	EventLoop* ioLoop = threadPool_->getNextLoop();
	char buf[64] = { 0 };
	snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
	++nextConnId_;
	std::string connName = name_ + buf;

	LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s \n",
		name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());

	sockaddr_in local;
	::bzero(&local, sizeof local);
	socklen_t addrlen = sizeof local;
	if (::getsockname(sockfd, (sockaddr*)&local, &addrlen) < 0) {
		LOG_ERROR("sockets::getLocalAddr");
	}
	InetAddress localAddr(local);

	// 根据连接成功的sockfd，创建TcpConnection连接对象。
	TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
	connections_[connName] = conn;
	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	conn->setWriteCompleteCallback(writeCompleteCallback_);
	conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
	ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));

}
 

void TcpServer::setThreadNum(int numThreads) {
	threadPool_->setThreadNum(numThreads);
}


void TcpServer::start() {
	if (started_++ == 0) { // 防止一个TcpServer对象被start多次
		threadPool_->start(threadInitCallback_); // 启动底层的线程池
		loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get())); // 即listen和设置sockfd为enableReading
	}
}


void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
	loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}


void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
	LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s \n", name_.c_str(), conn->name().c_str());

	connections_.erase(conn->name());
	EventLoop* ioLoop = conn->getLoop();
	ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}
