#pragma once
/*

*/

#include "noncopyable.h"
#include <string>

// 定义几个宏来调用log的函数来输出信息，就不需要让客户来创建Logger类。
// LOG_INFO("%s %d", arg1, arg2)。写宏的多行代码的时候，每一行后面都需要加一个"\"。
// __VA_ARGS__:用于在宏替换部分中，表示可变参数列表。
// #： 用来把参数转换成字符串。
// ##：用于将带参数的宏定义中将两个子串(token)联接起来，从而形成一个新的子串；
// 但它不可以是第一个或者最后一个子串。所谓的子串(token)就是指编译器能够识别的最小语法单元；
// ##__VA_ARGS__ 宏前面加上##的作用在于，当可变参数的个数为0时，这里的##起到把前面多余的","去掉的作用
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
	INFO, // 普通
	ERROR, // 错误
	FATAL, // core
	DEBUG, // 调试
};

class Logger
{
public:
	static Logger& instance(); // 获取日志唯一的实例对象
	void setLogLevel(int level); // 设置日志级别
	void log(std::string msg); // 写日志
private: // 系统变量定义的时候，是下划线在前面
	int logLevel_; // 日志级别。
};
