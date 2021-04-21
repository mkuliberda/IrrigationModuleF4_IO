#include "Logger.h"
#include "string.h"
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
void HAL_FatFs_Logger::writeLog(log_msg *_msg, FIL *log_file) {
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

