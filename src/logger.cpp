#include "Logger.h"
#include "string.h"
#include <algorithm>
#include <bitset>
#include "utilities.h"
#ifdef __cplusplus
 extern "C" {
#endif
#include "cmsis_os.h"
#ifdef __cplusplus
 }
#endif

osMutexId logger_mutex = NULL;
osMutexDef (logger_mutex);

/**
 * @brief 
 * 
 * @return HAL_FatFs_Logger& 
 */
HAL_FatFs_Logger& HAL_FatFs_Logger::createInstance(){
	if (logger_mutex == NULL) logger_mutex = osMutexCreate(osMutex(logger_mutex));
	static HAL_FatFs_Logger instance;
	return instance;
}

/**
 * @brief 
 * 
 * @param _msg 
 * @param log_file 
 */
void HAL_FatFs_Logger::writeLog(LogMsg *_msg, FIL *log_file) {
	if (valid){
		osMutexWait(logger_mutex, osWaitForever);
		char text[LOG_TEXT_LEN] = "";
		strncpy(text, _msg->text, _msg->len);
		if (f_open(log_file, file_path.c_str(), FA_WRITE | FA_OPEN_APPEND) == FR_OK) {
			f_printf(log_file, log_format, _msg->time.year, _msg->time.month, _msg->time.day, _msg->time.hours, _msg->time.minutes, _msg->time.seconds, _msg->time.milliseconds, reporter[_msg->reporter_id], text);
			while (f_close(log_file) != FR_OK);
		}
		osMutexRelease(logger_mutex);
	}
}

/**
 * @brief 
 * 
 * @param _msg 
 * @param log_file 
 * @param _reporter 
 */
void HAL_FatFs_Logger::writeLog(const std::string_view& _msg, FIL *log_file, const reporter_t& _reporter) {
	if (valid){
		RTC_TimeTypeDef rtc_time;
		RTC_DateTypeDef rtc_date;

		HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);

		uint16_t milliseconds = 1000 * (rtc_time.SecondFraction - rtc_time.SubSeconds) / (rtc_time.SecondFraction + 1);

		osMutexWait(logger_mutex, osWaitForever);
		if (f_open(log_file, file_path.c_str(), FA_WRITE | FA_OPEN_APPEND) == FR_OK) {
			std::string text{_msg};
			if (text.length() >= log_text_max_len){
				text = text.substr(text.length() - log_text_max_len + 1, log_text_max_len - 1);
			}
			f_printf(log_file, log_format, rtc_date.Year, rtc_date.Month,  rtc_date.Date, rtc_time.Hours, rtc_time.Minutes, rtc_time.Seconds, milliseconds, reporter[_reporter], text.c_str());
			while (f_close(log_file) != FR_OK);
		}
		osMutexRelease(logger_mutex);
	}
}

/**
 * @brief 
 * 
 * @param _log_text_max_len 
 */
void HAL_FatFs_Logger::setMsgMaxLength(const uint32_t & _log_text_max_len){
	if (_log_text_max_len > 0) log_text_max_len = _log_text_max_len;
	else valid = false;
}

/**
 * @brief 
 * 
 * @param _file_path 
 */
void HAL_FatFs_Logger::changeFilePath(const std::string_view& _file_path) {
	file_path = _file_path;    
}

void HAL_FatFs_Logger::accumulateLogs(osMailQId &_mail_box){
	osEvent evt;
	LogMsg *message = nullptr;
	do{
		evt = osMailGet(_mail_box, 1);
		if (evt.status == osEventMail){
			message = (LogMsg*)evt.value.p;
			char time_str[TIME_STR_LEN] = "";
			sprintf(time_str, time_format, message->time.year, message->time.month, message->time.day, message->time.hours, message->time.minutes, message->time.seconds, message->time.milliseconds);
			std::string_view str{time_str};
			logs.emplace_back(str, message);
		}
		osMailFree(_mail_box, message);
	}while(evt.status == osEventMail);
}

void HAL_FatFs_Logger::releaseLogsToFile(FIL *log_file){
	std::sort(logs.begin(), logs.end());
	for (const auto& log: logs){
		writeLog(log.second, log_file);
	}
	logs.clear();
}

uint8_t HAL_FatFs_Logger::publishLogMessage(std::string_view msg_txt, osMailQId &mail_box, const reporter_t &_reporter, const log_msg_prio_t &_msg_priority)
{
	std::bitset<8> errcode;
	/*******errcode**********
	 * 00000000
	 * ||||||||->(0) 1 if msg was too long and had to be shortened
	 * |||||||-->(1) 1 if osMailPut was unsuccessfull
	 * ||||||--->(2) 1 if
	 * |||||---->(3) 1 if
	 * ||||----->(4) 1 if
	 * |||------>(5) 1 if
	 * ||------->(6) 1 if
	 * |-------->(7) 1 if
	 *************************/
	RTC_TimeTypeDef rtc_time;
	RTC_DateTypeDef rtc_date;

	HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);
	const uint8_t _maxlen = LOG_TEXT_LEN;

	LogMsg *msg = (LogMsg*)osMailAlloc(mail_box, osWaitForever);

	msg->time.day = rtc_date.Date;
	msg->time.hours = rtc_time.Hours;
	msg->time.minutes = rtc_time.Minutes;
	msg->time.month = rtc_date.Month;
	msg->time.seconds = rtc_time.Seconds;
	msg->time.year = rtc_date.Year;
	msg->time.milliseconds = 1000 * (rtc_time.SecondFraction - rtc_time.SubSeconds) / (rtc_time.SecondFraction + 1);

	if (msg_txt.length() >= _maxlen){
		msg_txt = msg_txt.substr(msg_txt.length() - _maxlen + 1, _maxlen - 1);
		errcode.set(0, true);
	}
	msg->reporter_id = _reporter;
	msg->len = msg_txt.length() + 1;
	msg->priority = _msg_priority;
	msg_txt.copy(msg->text, msg_txt.length(), 0);
	msg->text[MINIMUM((std::size_t)(_maxlen - 1), msg_txt.length())] = '\0';

	if (osMailPut(mail_box, msg) != osOK){
		errcode.set(1, true);
	}

	return static_cast<uint8_t>(errcode.to_ulong());
}
