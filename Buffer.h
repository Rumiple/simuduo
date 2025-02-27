#pragma once


#include <vector>
#include <string>
#include <algorithm>

class Buffer
{
public:
	static const size_t kCheapPrepend = 8;
	static const size_t kInitialSize = 1024;

	explicit Buffer(size_t initialSize = kInitialSize)
		: buffer_(kCheapPrepend)
		, readerIndex_(kCheapPrepend)
		, writerIndex_(kCheapPrepend)
	{}

	size_t readableBytes() const {
		return writerIndex_ - readerIndex_;
	}

	size_t writableBytes() const {
		return buffer_.size() - writerIndex_;
	}

	size_t prependableBytes() const {
		return readerIndex_;
	}

	const char* peek() const { // 返回缓冲区中可读部分的起始地址
		return begin() + readerIndex_;
	}

	void retrieve(size_t len) {
		if (len < readableBytes()) {
			readerIndex_ += len;
		}
		else {
			retrieveAll();
		}
	}

	void ensureWriteableBytes(size_t len) {
		if (writableBytes() < len) {
			makeSpace(len); // 扩容至少len长度
		}
	}

	void append(const char* data, size_t len) {
		ensureWriteableBytes(len);
		std::copy(data, data + len, beginWrite());
		writerIndex_ += len;
	}

	char* beginWrite() {
		return begin() + writerIndex_;
	}

	const char* beginWrite() const {
		return begin() + writerIndex_;
	}

	ssize_t readFd(int fd, int* savedErrno); // 从fd上读取数据
	ssize_t writeFd(int fd, int* saveErrno);

	void retrieveAll() {
		readerIndex_ = writerIndex_ = kCheapPrepend;
	}

	std::string retrieveAllAsString() { // 把buffer里的数据转换成string返回
		return retrieveAsString(readableBytes());
	}

	std::string retrieveAsString(size_t len) {
		std::string result(peek(), len);
		retrieve(len); // 把缓冲区中选中的可读数据已经读取出来了，因此要更新readIndex_。
		return result;
	}

	
private:
	char* begin() {
		return &*buffer_.begin(); // 相当于it.operator*().operator&()
	}

	const char* begin() const {
		return &*buffer_.begin();
	}

	void makeSpace(size_t len) {
		if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
			buffer_.resize(writerIndex_ + len);
		}
		else {
			size_t readable = readableBytes();
			std::copy(begin() + readerIndex_,
				begin() + writerIndex_,
				begin() + kCheapPrepend);
			readerIndex_ = kCheapPrepend;
			writerIndex_ = readerIndex_ + readable;
		}
	}
	std::vector<char> buffer_;
	size_t readerIndex_;
	size_t writerIndex_;


};