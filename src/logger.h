#pragma once
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __logger_H
#define __logger_H

#include <memory>
#include <map>
#include <vector>
#include <string_view>

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

#define LOG_TEXT_LEN 40
#define TIME_FORMAT "%02d-%02d-%02d %02d:%02d:%02d.%03d"
#define TIME_FORMAT_LEN (sizeof(TIME_FORMAT))
#define LOG_FORMAT "%02d-%02d-%02d %02d:%02d:%02d.%03d %s: %s\n"
#define LOG_FORMAT_LEN (sizeof(LOG_FORMAT))
#define TIME_STR_LEN 22
#define LOG_FILE  "LOG.TXT" 

#define REPORTERS C(Sector1)C(Sector2)C(Sector3)C(Sector4)C(Watertank1)C(Task_SDCard)C(Task_Irrigation)C(Task_WirelessReceiver)C(Task_WirelessTransmitter)C(Task_SmsReceiver)C(Task_SmsTransmitter)C(Task_SysMonitor)C(Task_Gsm)C(Generic)
#define C(x) x,
enum reporter_t { REPORTERS TOP };
#undef C
#define C(x) #x,
const char * const reporter[] = { REPORTERS };

enum log_msg_prio_t : uint8_t {
	INFO,
	LOW,
	MEDIUM,
	HIGH,
	CRITICAL
};

struct LogTime {
	uint8_t hours{};
	uint8_t minutes{};
	uint8_t seconds{};
	uint8_t day{};
	uint8_t month{};
	uint8_t year{};
	uint16_t milliseconds{};
};

struct LogMsg {
	char 			text[LOG_TEXT_LEN]{};
	uint8_t			len{};
	reporter_t 		reporter_id{};
	log_msg_prio_t  priority{};
	LogTime		time{};
};

class HAL_FatFs_Logger
{
private:
	uint32_t log_text_max_len{ LOG_TEXT_LEN };
	bool valid{ true };
	std::string file_path{LOG_FILE};
	const TCHAR log_format[LOG_FORMAT_LEN] = LOG_FORMAT;
	const char time_format[TIME_FORMAT_LEN] = TIME_FORMAT;
	std::vector<std::pair<std::string_view, LogMsg*>> logs;

	HAL_FatFs_Logger() = default;

public:
	~HAL_FatFs_Logger() = default;
	HAL_FatFs_Logger(const HAL_FatFs_Logger&) = delete;
	HAL_FatFs_Logger& operator =(const HAL_FatFs_Logger&) = delete;

	static HAL_FatFs_Logger& createInstance();
	void writeLog(LogMsg *_msg, FIL *log_file);
	void writeLog(const std::string_view& _msg, FIL *log_file, const reporter_t& _reporter);
	void changeFilePath(const std::string_view& _file_path);
	void setMsgMaxLength(const uint32_t& _log_text_max_len);
	void accumulateLogs(osMailQId &_mail_box);
	void releaseLogsToFile(FIL *log_file);
	static uint8_t publishLogMessage(std::string_view msg_txt, osMailQId &mail_box, const reporter_t &_reporter, const log_msg_prio_t &_msg_priority);

};

#endif
