/*
 * scheduler.h
 *
 *  Created on: 05.09.2020
 *      Author: Mati
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <vector>
#include <string>
#include <time.h>
#include <string_view>

//"A001:20-05-01,20-09-30,MON,06-00-00,00-10-00";
#define ACTIVITY_LENGTH 44
#define ACTIVITY_ID_POS 1
#define ACTIVITY_ID_LEN 3
#define ACTIVITY_SYM_POS (ACTIVITY_ID_POS + ACTIVITY_ID_LEN)
#define ACTIVITY_SYM_LEN 1
#define ACTIVITY_BEGIN_YR_POS (ACTIVITY_SYM_POS + ACTIVITY_SYM_LEN)
#define ACTIVITY_BEGIN_YR_LEN 2
#define ACTIVITY_BEGIN_MO_POS 8
#define ACTIVITY_BEGIN_MO_LEN 2
#define ACTIVITY_BEGIN_DY_POS 11
#define ACTIVITY_BEGIN_DY_LEN 2
#define ACTIVITY_END_YR_POS 14
#define ACTIVITY_END_YR_LEN 2
#define ACTIVITY_END_MO_POS 17
#define ACTIVITY_END_MO_LEN 2
#define ACTIVITY_END_DY_POS 20
#define ACTIVITY_END_DY_LEN 2
#define ACTIVITY_DAY_OF_WEEK_POS 23
#define ACTIVITY_DAY_OF_WEEK_LEN 3
#define ACTIVITY_BEGIN_HR_POS 27
#define ACTIVITY_BEGIN_HR_LEN 2
#define ACTIVITY_BEGIN_MIN_POS 30
#define ACTIVITY_BEGIN_MIN_LEN 2
#define ACTIVITY_BEGIN_SEC_POS 33
#define ACTIVITY_BEGIN_SEC_LEN 2
#define ACTIVITY_DURATION_HR_POS 36
#define ACTIVITY_DURATION_HR_LEN 2
#define ACTIVITY_DURATION_MIN_POS 39
#define ACTIVITY_DURATION_MIN_LEN 2
#define ACTIVITY_DURATION_SEC_POS 42
#define ACTIVITY_DURATION_SEC_LEN 2
//E001:20-05-15,00-00-01,20-05-15,16-00-00
#define EXCEPTION_LENGTH 40
#define EXCEPTION_ID_POS 1
#define EXCEPTION_ID_LEN 3
#define EXCEPTION_SYM_POS 4
#define EXCEPTION_SYM_LEN 1
#define EXCEPTION_BEGIN_YR_POS 5
#define EXCEPTION_BEGIN_YR_LEN 2
#define EXCEPTION_BEGIN_MO_POS 8
#define EXCEPTION_BEGIN_MO_LEN 2
#define EXCEPTION_BEGIN_DY_POS 11
#define EXCEPTION_BEGIN_DY_LEN 2
#define EXCEPTION_BEGIN_HR_POS 14
#define EXCEPTION_BEGIN_HR_LEN 2
#define EXCEPTION_BEGIN_MIN_POS 17
#define EXCEPTION_BEGIN_MIN_LEN 2
#define EXCEPTION_BEGIN_SEC_POS 20
#define EXCEPTION_BEGIN_SEC_LEN 2
#define EXCEPTION_END_YR_POS 23
#define EXCEPTION_END_YR_LEN 2
#define EXCEPTION_END_MO_POS 26
#define EXCEPTION_END_MO_LEN 2
#define EXCEPTION_END_DY_POS 29
#define EXCEPTION_END_DY_LEN 2
#define EXCEPTION_END_HR_POS 32
#define EXCEPTION_END_HR_LEN 2
#define EXCEPTION_END_MIN_POS 35
#define EXCEPTION_END_MIN_LEN 2
#define EXCEPTION_END_SEC_POS 38
#define EXCEPTION_END_SEC_LEN 2

typedef struct {
	uint8_t weekday;
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
	uint8_t day;
	uint8_t month;
	uint8_t year;
}TimeStamp_t;

struct period_s{
	uint8_t day;
	uint8_t month;
	uint8_t year;
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
};

struct time_s{
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
};

struct activity_s{
	uint8_t id;
	uint8_t weekday;
	period_s begin;
	period_s end;
	time_s start;
	time_s duration;
};

struct exception_s{
	uint8_t id;
	period_s begin;
	period_s end;
};

class Scheduler final{

private:
	std::vector<activity_s> 				vActivities;
	std::vector<exception_s> 				vExceptions;
	const std::string_view					name;
	bool									is_active;
	bool									is_available;
	const uint8_t							activities_limit;
	const uint8_t							exceptions_limit;
	bool 									compare_time(const time_t &_time1, const time_t &_time2);


public:
	Scheduler(const std::string_view &_name, uint8_t _activities_limit = 255, uint8_t _exceptions_limit = 40):
	name(_name),
	is_active(false),
	is_available(false),
	activities_limit(_activities_limit),
	exceptions_limit(_exceptions_limit)
	{};

	const std::string_view& 				getName() const;
	bool& 									update(const TimeStamp_t &_timestamp);
	bool& 									isActive(void);
	bool 									isExceptionPeriod(const TimeStamp_t &_timestamp);
	bool& 									isAvailable(void);
	void									setAvailable(void);
	bool									addActivity(const struct activity_s &_activity);
	bool									addActivity(const char *_activity);
	bool									addException(const struct exception_s &_exception);
	bool									addException(const char *_exception);
	bool									addLine(const char *_line);
	std::vector<activity_s>&				getActivities(void);
	std::vector<exception_s>&				getExceptions(void);
	const uint8_t							getActivitiesCount(void) const;
	const uint8_t							getExceptionsCount(void) const;
	const uint8_t&							getActivitiesCountLimit(void) const;
	const uint8_t&							getExceptionsCountLimit(void) const;
	static activity_s						parseActivity(const char *_line);
	static exception_s						parseException(const char *_line);
};



#endif /* SCHEDULER_H_ */
