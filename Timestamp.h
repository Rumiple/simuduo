#pragma once

#include <iostream>
#include <string>

// �����ǰʱ��ķ����ǣ�Timestamp::now().toString()

class Timestamp
{
public:
	Timestamp();
	explicit Timestamp(int64_t microSecondsSinceEpoch); // Ҫ�������Timestamp���󣬶�������ʽת���ġ�
	static Timestamp now();
	std::string toString() const; // ��ֻ���ķ�ʽ��������ո�ʽ��
private:
	int64_t microSecondsSinceEpoch_;
};