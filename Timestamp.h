#pragma once

#include <iostream>
#include <string>

// 输出当前时间的方法是：Timestamp::now().toString()

class Timestamp
{
public:
	Timestamp();
	explicit Timestamp(int64_t microSecondsSinceEpoch); // 要求必须是Timestamp对象，而不是隐式转换的。
	static Timestamp now();
	std::string toString() const; // 以只读的方式输出年月日格式。
private:
	int64_t microSecondsSinceEpoch_;
};