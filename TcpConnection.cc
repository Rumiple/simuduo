#include "TcpConnection.h"
#include "Logger.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

#include <functional>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <netinet/tcp.h>
#include <string>


static EventLoop* CheckLoopNotNull(EventLoop* loop) {
	if (loop == nullptr) {
		LOG_FATAL("%s:%s:%d TcpConnection Loop is null! \n", __FILE__, __FUNCTION__, __LINE__);
	}
	return loop;
}

TcpConnection::TcpConnection(EventLoop* loop, const std::string& nameArg, int sockfd, 
	const InetAddress& localAddr, const InetAddress& peerAddr)
	: loop_(CheckLoopNotNull(loop))
	, name_(nameArg)
	, state_(kConnecting)
	, reading_(true)
	, socket_(new Socket(sockfd))
	, channel_(new Channel(loop, sockfd))
	, localAddr_(localAddr)
	, peerAddr_(peerAddr)
	, highWaterMark_(64 * 1024 * 1024) // 64M，用来限制当发送过快而对方接收过慢的情况。
{
	channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
	channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
	channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
	channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

	LOG_INFO("TcpConnection::ctor[%s] at fd=%d\n", name_.c_str(), sockfd);
	socket_->setKeepAlive(true);
}


TcpConnection::~TcpConnection() {
	LOG_INFO("TcpConnection::dtor[%s] at fd:%d state=%d \n", name_.c_str(), channel_->fd(), (int)state_);
}


void TcpConnection::handleRead(Timestamp receiveTime) {
	int savedErrno = 0;
	ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
	if (n > 0) {
		// 已建立连接的用户，有可读事件发生了调用用户传入的回调操作
		messageCallback_(shared_from_this(), &inputBuffer_, receiveTime); // shared_from_this()是获取当前TcpConnection对象的智能指针
	}
	else if (n == 0) { // 表示连接断开。
		handleClose();
	} 
	else {
		errno = savedErrno;
		LOG_ERROR("TcpConnection::handleRead");
		handleError();
	}
}

 
void TcpConnection::handleWrite() {
	if (channel_->isWriting()) {
		int savedErrno = 0;
		ssize_t n = outputBuffer_.writeFd(channel_->fd(), &savedErrno);
		if (n > 0) {
			outputBuffer_.retrieve(n);
			if (outputBuffer_.readableBytes() == 0) {
				channel_->disableWriting();
				if (writeCompleteCallback_) {
					loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
				}
				if (state_ == kDisconnecting) {
					shutdownInLoop();
				}
			}
		}
		else {
			LOG_ERROR("TcpConnection::handleWrite");
		}
	}
	else {
		LOG_ERROR("TcpConnection::handleWrite fd=%d is down, no more writing \n", channel_->fd());
	}
}


void TcpConnection::handleClose() {
	LOG_INFO("TcpConnection::handleClose fd=%d state=%d \n", channel_->fd(), (int)state_);
	setState(kDisconnected);
	channel_->disableAll();

	TcpConnectionPtr connPtr(shared_from_this());
	connectionCallback_(connPtr); // 执行连接关闭的回调
	closeCallback_(connPtr); // 关闭连接的回调

}


void TcpConnection::handleError() {
	int optval;
	socklen_t optlen = sizeof optval;
	int err = 0;
	if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
		err = errno;
	}
	else {
		err = optval;
	}
	LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d \n", name_.c_str(), err);
}


void TcpConnection::send(const std::string& buf) {
	if (state_ == kConnected) {
		if (loop_->isInLoopThread()) {
			sendInLoop(buf.c_str(), buf.size());
		}
		else {
			loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size()));
		}
	}
}


void TcpConnection::sendInLoop(const void* data, size_t len) {
	ssize_t nwrote = 0;
	size_t remaining = len;
	bool faultError = false;
	if (state_ == kDisconnected) {
		LOG_ERROR("disconnected, give up writing!");
		return;
	}

	if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) { // 
		nwrote = ::write(channel_->fd(), data, len);
		if (nwrote >= 0) {
			remaining = len - nwrote;
			if (remaining == 0 && writeCompleteCallback_) { // 数据全部发送完毕。
				loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
			}
		}
		else {
			nwrote = 0;
			if (errno != EWOULDBLOCK) { // EWOULDBLOCK表示的含义是由于非阻塞没有数据，属于正常的返回
				LOG_ERROR("TcpConnection::sendInLoop");
				if (errno == EPIPE || errno == ECONNRESET) { // SIGPIPE RESET
					faultError = true;
				}
			}
		}
	}
	// 数据一次没有全部发送完，剩余的数据保留坐在了缓冲区当中，给channel注册epollout事件，poller通过相应的sock-channel
	// 调用writeCallback也就是TcpConnection::handlWrite把发送缓冲区中的数据全部发送出去。
	if (!faultError && remaining > 0) {
		size_t oldLen = outputBuffer_.readableBytes(); // 目前发送缓冲区剩余的待发送数据的长度
		if (oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_) {
			loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
		}
		outputBuffer_.append((char*)data + nwrote, remaining);
		if (!channel_->isWriting()) {
			channel_->enableWriting(); // 重新注册写事件。
		}
	}
}


void TcpConnection::connectEstablished() {
	setState(kConnected);
	channel_->tie(shared_from_this());
	channel_->enableReading();
	connectionCallback_(shared_from_this());
}


void TcpConnection::connectDestroyed() {
	if (state_ == kConnected) {
		setState(kDisconnected);
		channel_->disableAll();
		connectionCallback_(shared_from_this());
	}
	channel_->remove();
}


void TcpConnection::shutdown() {
	if (state_ == kConnected) {
		setState(kDisconnecting);
		loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
	}
}


void TcpConnection::shutdownInLoop() {
	if (!channel_->isWriting()) { // 说明outputBuffer中的数据已经全部发送完了
		socket_->shutdownWrite(); // 关闭写端
	}
}