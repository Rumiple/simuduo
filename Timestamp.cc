#include "Timestamp.h"
#include <time.h>

Timestamp::Timestamp() : microSecondsSinceEpoch_(0) {}


Timestamp::Timestamp(int64_t microSecondSinceEpoch)
	: microSecondsSinceEpoch_(microSecondSinceEpoch) {}


Timestamp Timestamp::now() {
	return Timestamp(time(NULL)); // time返回当前日历时间。自1970-01-01到目前的秒数。
}


std::string Timestamp::toString() const {
	char buf[128] = { 0 };
	tm* tm_time = localtime(&microSecondsSinceEpoch_); // localtime用于将时间戳（time_t类型）转换为本地时间的结构体.
	snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d",
		tm_time->tm_year + 1900,
		tm_time->tm_mon + 1,
		tm_time->tm_mday,
		tm_time->tm_hour,
		tm_time->tm_min,
		tm_time->tm_sec);
	return buf;
}


