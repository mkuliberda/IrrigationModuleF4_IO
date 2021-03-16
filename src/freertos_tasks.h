/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __freertos_tasks_H
#define __freertos_tasks_H


#include "scheduler.h"
#include "plants.h"

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "adc.h"
#include "fatfs.h"
#include "iwdg.h"
#include "rtc.h"
#include "sdio.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "sd_diskio.h"
#include "stm32f4xx_hal_rtc.h"

#define LOG_TEXT_LEN 26
#define LOG_FORMAT "%02d-%02d-%02d %02d:%02d:%02d %s: %s\n"
#define SECTORS_AMOUNT 4

#define REPORTERS C(Sector1)C(Sector2)C(Sector3)C(Sector4)C(Task_SDCard)C(Task_Irrigation)C(Task_Wireless)C(Task_SysMonitor)C(Generic)
#define C(x) x,
enum reporter_t { REPORTERS TOP };
#undef C
#define C(x) #x,
const char * const reporter[] = { REPORTERS };

struct activity_msg{
	uint8_t sector_nbr;
	activity_s activity;
};

struct exception_msg{
	uint8_t sector_nbr;
	exception_s exception;
};

struct plant_msg{
	char	name[PLANT_NAME_LEN] = "";
	uint8_t sector_nbr;
	bool	rain_exposed;
	uint8_t type;
};

struct log_time{
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
	uint8_t day;
	uint8_t month;
	uint8_t year;
};

struct log_msg{
	char 			text[LOG_TEXT_LEN];
	uint8_t			len;
	reporter_t 		reporter_id;
	log_time		time;
};

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );
/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );
void MX_FREERTOS_Init(void);


#ifdef __cplusplus
}
#endif
#endif
