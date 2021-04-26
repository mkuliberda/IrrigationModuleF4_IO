/*
 * utilities.h
 *
 *  Created on: 24.12.2019
 *      Author: Mati
*/

#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <string>
#include <sstream>
#include "rtc.h"

//TODO: defines deprecated, remove in future
#define MINIMUM(a,b)            (((a) < (b)) ? (a) : (b))
#define MAXIMUM(a,b)            (((a) < (b)) ? (b) : (a))

template <typename T>
T SmallerOfTwo(T& first, T& second) { return first < second ? first : second; };

template <typename T>
T BiggerOfTwo(T& first, T& second) { return first > second ? first : second; };

template<class T>
constexpr const T& clamp( const T& v, const T& lo, const T& hi )
{
    assert( !(hi < lo) );
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

namespace patch
{
    template < typename T > 
    std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}

inline void setRtcFromStr(const std::string& _str){
    RTC_TimeTypeDef rtc_time;
	RTC_DateTypeDef rtc_date;

    rtc_time.TimeFormat = 0;
    rtc_time.Hours = atoi(_str.substr(34,2).c_str());
    rtc_time.Minutes = atoi(_str.substr(36,2).c_str());
    rtc_time.Seconds = atoi(_str.substr(38,2).c_str());
    //rtc_time.DayLightSaving = 0;
    std::string_view weekday = _str.substr(31,3);
    if(weekday == "Mon"){
        rtc_date.WeekDay = RTC_WEEKDAY_MONDAY;
    }
    else if(weekday == "Tue"){
        rtc_date.WeekDay = RTC_WEEKDAY_TUESDAY;
    }
    else if(weekday == "Wed"){
        rtc_date.WeekDay = RTC_WEEKDAY_WEDNESDAY;
    }
    else if(weekday == "Thu"){
        rtc_date.WeekDay = RTC_WEEKDAY_THURSDAY;
    }
    else if(weekday == "Fri"){
        rtc_date.WeekDay = RTC_WEEKDAY_FRIDAY;
    }
    else if(weekday == "Sat"){
        rtc_date.WeekDay = RTC_WEEKDAY_SATURDAY;
    }
    else if(weekday == "Sun"){
        rtc_date.WeekDay = RTC_WEEKDAY_SUNDAY;
    }
    rtc_date.Date = atoi(_str.substr(29,2).c_str());
    rtc_date.Month = atoi(_str.substr(27,2).c_str());
    rtc_date.Year = atoi(_str.substr(25,2).c_str());
    HAL_RTC_SetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);
}

#endif /* UTILITIES_H_ */
