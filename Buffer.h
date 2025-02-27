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

	const char* peek() const { // ���ػ������пɶ����ֵ���ʼ��ַ
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
			makeSpace(len); // ��������len����
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

	ssize_t readFd(int fd, int* savedErrno); // ��fd�϶�ȡ����
	ssize_t writeFd(int fd, int* saveErrno);

	void retrieveAll() {
		readerIndex_ = writerIndex_ = kCheapPrepend;
	}

	std::string retrieveAllAsString() { // ��buffer�������ת����string����
		return retrieveAsString(readableBytes());
	}

	std::string retrieveAsString(size_t len) {
		std::string result(peek(), len);
		retrieve(len); // �ѻ�������ѡ�еĿɶ������Ѿ���ȡ�����ˣ����Ҫ����readIndex_��
		return result;
	}

	
private:
	char* begin() {
		return &*buffer_.begin(); // �൱��it.operator*().operator&()
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