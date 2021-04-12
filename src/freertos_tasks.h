/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __freertos_tasks_H
#define __freertos_tasks_H


#include "../lib/scheduler/scheduler.h"
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

#define SECTORS_AMOUNT 4
//{"NTP999>SYS000":{"CTD":"200914Mon185950"}}
//$RPI001>SEC000:"CMD":001\n
//$RPI001>SEC001:"REQ":003\n
//$GOO001>SEC999:"CMD":001\n
#define EXT_TIME_FORMAT "{\"NTP999>SYS000\":{\"CTD\":\"200914Mon185950\"}}" //Current Time Date
#define EXT_TIME_STR_LEN (sizeof(EXT_TIME_FORMAT)-1)

struct activity_msg{
	uint8_t sector_nbr{};
	activity_s activity{};
};

struct exception_msg{
	uint8_t sector_nbr{};
	exception_s exception{};
};

struct plant_config_msg{
	char	name[PLANT_NAME_LEN] = "";
	uint8_t sector_nbr{};
	bool	rain_exposed{};
	uint8_t type{};
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
