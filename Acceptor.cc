#include "Acceptor.h"
#include "Logger.h"
#include "InetAddress.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>

static int createNonblocking() {
	int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
	if (sockfd < 0) {
		LOG_FATAL("%s:%s:%d listen socket create err:$d \n", __FILE__, __FUNCTION__, __LINE__, errno); // 出错的时候会打印文件名、函数名、函数.
	}
	return sockfd;

}

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport) 
	: loop_(loop)
	, acceptSocket_(createNonblocking())
	, acceptChannel_(loop, acceptSocket_.fd())
	, listenning_(false)
{
	acceptSocket_.setreuseAddr(true);
	acceptSocket_.setReusePort(true);
	acceptSocket_.bindAddress(listenAddr);
	// 由TcpServer::start() ----> Acceptor::listen()，如果有新用户连接，需要执行一个回调
	acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}


Acceptor::~Acceptor() {
	acceptChannel_.disableAll();
	acceptChannel_.remove();
}


void Acceptor::listen() {
	listenning_ = true;
	acceptSocket_.listen();
	acceptChannel_.enableReading();
}


void Acceptor::handleRead() { // listenfd由事件发生即有新用户连接，触发handleRead
	InetAddress peerAddr;
	int connfd = acceptSocket_.accept(&peerAddr);
	if (connfd >= 0) {
		if (newConnectionCallback_) {
			newConnectionCallback_(connfd, peerAddr); // 轮询找到subloop并唤醒分发当前新客户端的channel
		}
		else {
			::close(connfd);
		}
	}
	else {
		LOG_ERROR("%s:%s:%d accept create err:$d \n", __FILE__, __FUNCTION__, __LINE__, errno);
		if (errno == EMFILE) { // sockfd资源不足
			LOG_ERROR("%s:%s:%d sockfd reached limit\n", __FILE__, __FUNCTION__, __LINE__);
		}
	}

}
