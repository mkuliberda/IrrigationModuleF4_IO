#include "HAL_UART_ESP01S_MsgParser.h"
#include <string>
#include "stm32f4xx_hal.h"
#include "rtc.h"

bool HAL_UART_ESP01S_MsgParser::parseString(const std::string& _str){
    
    if(_str.substr(2,13) == "NTP999>SYS000"){
        if (_str.substr(19,3) == "CTD"){
            setRtc(_str);
            return true;
        }

    }
    return false;
}

void HAL_UART_ESP01S_MsgParser::setRtc(const std::string& _str){
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