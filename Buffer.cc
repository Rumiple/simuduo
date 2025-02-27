#include "Buffer.h"

#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>

/*
* struct iovec {
*	void* uov_base; // 起始地址
*	size_t iov_len; // 长度
* 
*/



ssize_t Buffer::readFd(int fd, int* saveErrno) {  // Poller工作在LT模式
	char extrabuf[65536] = { 0 }; // 备用的缓冲区，64k, 栈上的内存空间
	struct iovec vec[2]; // 一块buffer缓冲区，一块备用的缓冲区
	const size_t writable = writableBytes();
	vec[0].iov_base = begin() + writerIndex_;
	vec[0].iov_len = writable;

	vec[1].iov_base = extrabuf;
	vec[1].iov_len = sizeof extrabuf;

	const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
	const ssize_t n = ::readv(fd, vec, iovcnt);
	if (n < 0) {
		*saveErrno = errno;
	}
	else if (n <= writable) {
		writerIndex_ += n;
	}
	else {
		writerIndex_ = buffer_.size();
		append(extrabuf, n - writable); // 把剩下数据写入备用空间
	}
	return n;


}


ssize_t Buffer::writeFd(int fd, int* saveErrno) {
	ssize_t n = ::write(fd, peek(), readableBytes());
	if (n < 0) {
		*saveErrno = errno;
	}
	return n;
}

