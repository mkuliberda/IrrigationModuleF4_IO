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
#include "Messages.h"

//TODO: defines deprecated, remove in future
#define MINIMUM(a,b)            (((a) < (b)) ? (a) : (b))
#define MAXIMUM(a,b)            (((a) < (b)) ? (b) : (a))

template <typename T1, typename T2>
T1 SmallerOfTwo(T1& first, T2& second) { return first < second ? first : second; };

template <typename T1, typename T2>
T1 BiggerOfTwo(T1& first, T2& second) { return first > second ? first : second; };

template<typename T>
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

inline void setRtcFromIncoming(const IncomingMessage& _msg){
    RTC_TimeTypeDef rtc_time;
	RTC_DateTypeDef rtc_date;
//ymdwhms
    rtc_time.TimeFormat = 0;
    rtc_date.Year = static_cast<uint8_t>(_msg.msg_value1);
    rtc_date.Month = static_cast<uint8_t>(_msg.msg_value2);
    rtc_date.Date = static_cast<uint8_t>(_msg.msg_value3);
    rtc_date.WeekDay = static_cast<uint8_t>(_msg.msg_value4);
    rtc_time.Hours = static_cast<uint8_t>(_msg.msg_value5);
    rtc_time.Minutes = static_cast<uint8_t>(_msg.msg_value6);
    rtc_time.Seconds = static_cast<uint8_t>(_msg.msg_value7);
    //rtc_time.DayLightSaving = 0;

    HAL_RTC_SetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);
}

#endif /* UTILITIES_H_ */
