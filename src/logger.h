#pragma once
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __logger_H
#define __logger_H

#include <memory>

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "fatfs.h"
#include "sdio.h"
#include "sd_diskio.h"
#include "stm32f4xx_hal_rtc.h"

#ifdef __cplusplus
}
#endif

#define LOG_TEXT_LEN 26
#define LOG_FORMAT "%02d-%02d-%02d %02d:%02d:%02d %s: %s\n"
#define LOG_FILE  "LOG.TXT" 

#define REPORTERS C(Sector1)C(Sector2)C(Sector3)C(Sector4)C(Task_SDCard)C(Task_Irrigation)C(Task_Wireless)C(Task_SysMonitor)C(Generic)
#define C(x) x,
enum reporter_t { REPORTERS TOP };
#undef C
#define C(x) #x,
const char * const reporter[] = { REPORTERS };

struct log_time {
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
	uint8_t day;
	uint8_t month;
	uint8_t year;
};

struct log_msg {
	char 			text[LOG_TEXT_LEN];
	uint8_t			len;
	reporter_t 		reporter_id;
	log_time		time;
};


//TODO: add thread safety
class HAL_FatFs_Logger
{
private:
	uint32_t log_text_max_len{ LOG_TEXT_LEN };
	bool valid{ true };
	std::string file_path{LOG_FILE};
	const TCHAR log_format[38] = LOG_FORMAT;

	HAL_FatFs_Logger() = default;

public:
	~HAL_FatFs_Logger() = default;
	HAL_FatFs_Logger(const HAL_FatFs_Logger&) = delete;
	HAL_FatFs_Logger& operator =(const HAL_FatFs_Logger&) = delete;

	static HAL_FatFs_Logger& createInstance();
	void writeLog(log_msg *_msg, FIL *log_file);
	void writeLog(const std::string_view& _msg, FIL *log_file, const reporter_t& _reporter);
	void changeFilePath(const std::string_view& _file_path);
	void setMsgMaxLength(const uint32_t& _log_text_max_len);

};

#endif
