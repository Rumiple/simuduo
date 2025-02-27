#pragma once
/*

*/

#include "noncopyable.h"
#include <string>

// ���弸����������log�ĺ����������Ϣ���Ͳ���Ҫ�ÿͻ�������Logger�ࡣ
// LOG_INFO("%s %d", arg1, arg2)��д��Ķ��д����ʱ��ÿһ�к��涼��Ҫ��һ��"\"��
// __VA_ARGS__:�����ں��滻�����У���ʾ�ɱ�����б�
// #�� �����Ѳ���ת�����ַ�����
// ##�����ڽ��������ĺ궨���н������Ӵ�(token)�����������Ӷ��γ�һ���µ��Ӵ���
// �����������ǵ�һ���������һ���Ӵ�����ν���Ӵ�(token)����ָ�������ܹ�ʶ�����С�﷨��Ԫ��
// ##__VA_ARGS__ ��ǰ�����##���������ڣ����ɱ�����ĸ���Ϊ0ʱ�������##�𵽰�ǰ������","ȥ��������
#define LOG_INFO(logmsgFormat, ...) \
	do \
	{ \
		Logger &logger = Logger::instance(); \
		logger.setLogLevel(INFO); \
		char buf[1024] = { 0 }; \
		snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
		logger.log(buf); \
	} while (0)

#define LOG_ERROR(logmsgFormat, ...) \
	do \
	{ \
		Logger &logger = Logger::instance(); \
		logger.setLogLevel(INFO); \
		char buf[1024] = { 0 }; \
		snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
		logger.log(buf); \
	} while (0)

#define LOG_FATAL(logmsgFormat, ...) \
	do \
	{ \
		Logger &logger = Logger::instance(); \
		logger.setLogLevel(INFO); \
		char buf[1024] = { 0 }; \
		snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
		logger.log(buf); \
		exit(-1); \
	} while (0)


#ifdef MUDEBUG
#define LOG_DEBUG(logmsgFormat, ...) \
	do \
	{ \
		Logger &logger = Logger::instance(); \
		logger.setLogLevel(INFO); \
		char buf[1024] = { 0 }; \
		snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
		logger.log(buf); \
	} while (0)
#else
	#define LOG_DEBUG(logmsgFormat, ...)
#endif

enum LogLevel
{
	INFO, // ��ͨ
	ERROR, // ����
	FATAL, // core
	DEBUG, // ����
};

class Logger
{
public:
	static Logger& instance(); // ��ȡ��־Ψһ��ʵ������
	void setLogLevel(int level); // ������־����
	void log(std::string msg); // д��־
private: // ϵͳ���������ʱ�����»�����ǰ��
	int logLevel_; // ��־����
};
