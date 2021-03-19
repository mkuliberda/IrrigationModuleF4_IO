
#include "freertos_tasks.h"
#include "freertos_memory.h"
//#include "printf_retarget.h"
#include <array>
#include <cstring>
#include "utilities.h"
#include <algorithm>
#include "watertank.h"
#include "pumps.h"
#include "sector.h"
#include "sensors.h"
#include "logger.h"


osThreadId SysMonitorTaskHandle;
osThreadId SDCardTaskHandle;
osThreadId WirelessCommTaskHandle;
osThreadId IrrigationControlTaskHandle;

osMailQId activities_box;
osMailQDef(activities_box, 42, activity_msg);
osMailQId exceptions_box;
osMailQDef(exceptions_box, 6, exception_msg);
osMailQId plants_box;
osMailQDef(plants_box, 20, plant_msg);
osMailQId sys_logs_box;
osMailQDef(sys_logs_box, 10, log_msg);
osMailQId irg_logs_box;
osMailQDef(irg_logs_box, 10, log_msg);
osMailQId wls_logs_box;
osMailQDef(wls_logs_box, 10, log_msg);


void SysMonitorTask(void const * argument);
void SDCardTask(void const *argument);
void WirelessCommTask(void const *argument);
void IrrigationControlTask(void const *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* USER CODE BEGIN GET_TIMER_TASK_MEMORY */
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
  *ppxTimerTaskStackBuffer = &xTimerStack[0];
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
  /* place for user code */
}

uint8_t publishLogMessage(std::string_view msg_txt, osMailQId &mail_box, const reporter_t &_reporter, const uint8_t &_maxlen)
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

	log_msg *msg = (log_msg*)osMailAlloc(mail_box, osWaitForever);

	msg->time.day = rtc_date.Date;
	msg->time.hours = rtc_time.Hours;
	msg->time.minutes = rtc_time.Minutes;
	msg->time.month = rtc_date.Month;
	msg->time.seconds = rtc_time.Seconds;
	msg->time.year = rtc_date.Year;

	if (msg_txt.length() >= _maxlen){
		msg_txt = msg_txt.substr(msg_txt.length() - _maxlen + 1, _maxlen - 1);
		errcode.set(0, true);
	}
	msg->reporter_id = _reporter;
	msg->len = msg_txt.length() + 1;
	msg_txt.copy(msg->text, msg_txt.length(), 0);
	msg->text[MINIMUM((std::size_t)(_maxlen - 1), msg_txt.length())] = '\0';

	if (osMailPut(mail_box, msg) != osOK){
		errcode.set(1, true);
	}

	return static_cast<uint8_t>(errcode.to_ulong());
}

osEvent updateSectorsActivities(Scheduler *schedule, osMailQId &mail_box)
{
	osEvent evt;
	activity_msg *msg = nullptr;

	do{
		evt = osMailGet(mail_box, 1);
		if (evt.status == osEventMail){
			msg = (activity_msg*)evt.value.p;
			switch (msg->sector_nbr){
			case 0:
				schedule[0].addActivity(msg->activity);
				break;
			case 1:
				schedule[1].addActivity(msg->activity);
				break;
			case 2:
				schedule[2].addActivity(msg->activity);
				break;
			case 3:
				schedule[3].addActivity(msg->activity);
				break;
			default:
				break;
			}
		}
		osMailFree(mail_box, msg);
	}while(evt.status == osEventMail);

	return evt;
}

osEvent updateSectorsExceptions(Scheduler *schedule, osMailQId &mail_box)
{
	osEvent evt;
	exception_msg *msg = nullptr;

	do{
		evt = osMailGet(mail_box, 1);
		if (evt.status == osEventMail){
			msg = (exception_msg*)evt.value.p;
			switch (msg->sector_nbr){
			case 0:
				schedule[0].addException(msg->exception);
				break;
			case 1:
				schedule[1].addException(msg->exception);
				break;
			case 2:
				schedule[2].addException(msg->exception);
				break;
			case 3:
				schedule[3].addException(msg->exception);
				break;
			default:
				break;
			}
		}
		osMailFree(mail_box, msg);
	}while(evt.status == osEventMail);

	return evt;
}


/* USER CODE END GET_TIMER_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(sysmonitorTask, SysMonitorTask, osPriorityRealtime, 0, 30*configMINIMAL_STACK_SIZE);
  SysMonitorTaskHandle = osThreadCreate(osThread(sysmonitorTask), NULL);

  osThreadDef(sdcardTask, SDCardTask, osPriorityNormal, 0, 80*configMINIMAL_STACK_SIZE);
  SDCardTaskHandle = osThreadCreate(osThread(sdcardTask), NULL);

  osThreadDef(wirelessTask, WirelessCommTask, osPriorityNormal, 0, 20*configMINIMAL_STACK_SIZE);
  WirelessCommTaskHandle = osThreadCreate(osThread(wirelessTask), NULL);

  osThreadDef(irrigationTask, IrrigationControlTask, osPriorityHigh, 0, 60*configMINIMAL_STACK_SIZE);
  IrrigationControlTaskHandle = osThreadCreate(osThread(irrigationTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void SysMonitorTask(void const * argument)
{
	size_t min_heap_size = 0;
	size_t prev_min_heap_size = 0;
	TaskStatus_t TaskStatus;
	configSTACK_DEPTH_TYPE SDCardPrevStackHighWaterMark = 0;
	configSTACK_DEPTH_TYPE WirelessCommPrevStackHighWaterMark = 0;
	configSTACK_DEPTH_TYPE IrrigationControlPrevStackHighWaterMark = 0;
	BaseType_t FreeStackSpace = 1;
	eTaskState State = eInvalid;
	sys_logs_box = osMailCreate(osMailQ(sys_logs_box), osThreadGetId());

	publishLogMessage("Sys monitor task started", sys_logs_box, reporter_t::Task_SysMonitor, LOG_TEXT_LEN);

	for(;;)
	{
    	min_heap_size = xPortGetMinimumEverFreeHeapSize();
    	if (min_heap_size != prev_min_heap_size){
    		prev_min_heap_size = min_heap_size;
    		std::string_view msg = "Min HEAP size:" + patch::to_string(min_heap_size) + "b";
    		publishLogMessage(msg, sys_logs_box, reporter_t::Task_SysMonitor, LOG_TEXT_LEN);
    	}
    	vTaskGetInfo(SDCardTaskHandle, &TaskStatus, FreeStackSpace, State);
    	if (TaskStatus.usStackHighWaterMark != SDCardPrevStackHighWaterMark){
    		std::string_view msg =  "SDCardTask HWMark:" + patch::to_string(TaskStatus.usStackHighWaterMark * 2) + "b";
    		SDCardPrevStackHighWaterMark = TaskStatus.usStackHighWaterMark;
    		publishLogMessage(msg, sys_logs_box, reporter_t::Task_SysMonitor, LOG_TEXT_LEN);
    	}
    	vTaskGetInfo(WirelessCommTaskHandle, &TaskStatus, FreeStackSpace, State);
    	if (TaskStatus.usStackHighWaterMark != WirelessCommPrevStackHighWaterMark){
    		std::string_view msg =  "WlsCom HWMark:"+ patch::to_string(TaskStatus.usStackHighWaterMark * 2) + "b";
    		WirelessCommPrevStackHighWaterMark = TaskStatus.usStackHighWaterMark;
    		publishLogMessage(msg, sys_logs_box, reporter_t::Task_SysMonitor, LOG_TEXT_LEN);
    	}
    	vTaskGetInfo(IrrigationControlTaskHandle, &TaskStatus, FreeStackSpace, State);
    	if (TaskStatus.usStackHighWaterMark != IrrigationControlPrevStackHighWaterMark){
    		std::string_view msg =  "IrrCtrl HWMark:"+ patch::to_string(TaskStatus.usStackHighWaterMark * 2) + "b";
    		IrrigationControlPrevStackHighWaterMark = TaskStatus.usStackHighWaterMark;
    		publishLogMessage(msg, sys_logs_box, reporter_t::Task_SysMonitor, LOG_TEXT_LEN);
    	}
		osDelay(2000);
	}
}

void SDCardTask(void const *argument)
{
    FIL log_file;
	FIL config_file;
	char logical_drive[4] = {0, 0, 0, 0};   /* SD logical drive path */
	FATFS file_system;    /* File system object for SD logical drive */
	DIR directory;
	FILINFO file_info;
	char cwd_buffer[80] = "/";

	HAL_FatFs_Logger &logger = HAL_FatFs_Logger::createInstance();

	const std::array<std::string_view, 4> config_file_candidates = {"SECTOR1.TXT", "SECTOR2.TXT", "SECTOR3.TXT",  "SECTOR4.TXT"};
	bool mount_success = false;
	activity_msg *activity = nullptr;
	activities_box = osMailCreate(osMailQ(activities_box), osThreadGetId());
	exception_msg *exception = nullptr;
	exceptions_box = osMailCreate(osMailQ(exceptions_box), osThreadGetId());
	plant_msg *plant = nullptr;
	plants_box = osMailCreate(osMailQ(plants_box), osThreadGetId());
	osEvent evt;
	log_msg *sys_message = nullptr;
	log_msg *irg_message = nullptr;
	log_msg *wls_message = nullptr;

	if(f_mount(&file_system, logical_drive, 1) == FR_OK){
		mount_success = true;
		logger.writeLog("SDCard mount successful!", &log_file, reporter_t::Task_SDCard);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);

		if (f_opendir(&directory, cwd_buffer) == FR_OK){
			do{
				// Read an entry from the directory.
				// Check for error and break if there is a problem.
				if( f_readdir(&directory, &file_info) != FR_OK){
					HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
					break;
				}

				for(auto const &file: config_file_candidates){

					std::string sdcard_sched_filename(file_info.fname);

					if (file == sdcard_sched_filename){
						uint8_t sector_nbr = atoi(sdcard_sched_filename.substr(6,1).c_str()) - 1;
						/* Open a text file */
						if (f_open(&config_file, file_info.fname, FA_READ) == FR_OK){
							char config_line[43] = "";
							while (f_gets(config_line, sizeof(config_line), &config_file)){
								if(config_line[0] == 'A'){
									activity = (activity_msg*)osMailAlloc(activities_box, osWaitForever);
									activity->sector_nbr = sector_nbr;
									activity->activity = Scheduler::parseActivity(config_line);
									while (osMailPut(activities_box, activity) != osOK);
								}
								else if(config_line[0] == 'E'){
									exception = (exception_msg*)osMailAlloc(exceptions_box, osWaitForever);
									exception->sector_nbr = sector_nbr;
									exception->exception = Scheduler::parseException(config_line);
									while (osMailPut(exceptions_box, exception) != osOK);
								}
								else if(config_line[0] == 'P'){	//format P:Y,1,Pelargonia...
									plant = (plant_msg*)osMailAlloc(plants_box, osWaitForever);
									plant->sector_nbr = sector_nbr;
									const std::string str(config_line);
									plant->rain_exposed = str.substr(2,1) == "Y" ? true : false;
									plant->type = atoi(str.substr(4,1).c_str());
									str.copy(plant->name, MINIMUM(str.length() - 6, PLANT_NAME_LEN-1), 6);
									while (osMailPut(plants_box, plant) != osOK);
								}
							}
							while (f_close(&config_file) != FR_OK);
						}
						else{
							HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
						}
					}
				}
			// If the file name is blank, then this is the end of the listing.
			}while(file_info.fname[0]);
		}
	}

    for( ;; )
    {
		if(mount_success){
			do{
				evt = osMailGet(sys_logs_box, 1);
				if (evt.status == osEventMail){
					sys_message = (log_msg*)evt.value.p;
					logger.writeLog(sys_message, &log_file);
				}
				osMailFree(sys_logs_box, sys_message);
			}while(evt.status == osEventMail);

			do{
				evt = osMailGet(irg_logs_box, 1);
				if (evt.status == osEventMail){
					irg_message = (log_msg*)evt.value.p;
					logger.writeLog(irg_message, &log_file);
				}
				osMailFree(irg_logs_box, irg_message);
			}while(evt.status == osEventMail);

			do{
				evt = osMailGet(wls_logs_box, 1);
				if (evt.status == osEventMail){
					wls_message = (log_msg*)evt.value.p;
					logger.writeLog(wls_message, &log_file);
				}
				osMailFree(wls_logs_box, wls_message);
			}while(evt.status == osEventMail);
		}

    	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
    	osDelay(200);
    }
}


void WirelessCommTask(void const *argument)
{
	wls_logs_box = osMailCreate(osMailQ(wls_logs_box), osThreadGetId());

	RTC_TimeTypeDef rtc_time;
	RTC_DateTypeDef rtc_date;

	//TODO: set time based on wireless communication from external computer
	rtc_time.Hours = 18;
	rtc_time.Minutes = 59;
	rtc_time.Seconds = 50;
	rtc_time.TimeFormat = 0;
	rtc_time.DayLightSaving = 0;
	rtc_date.WeekDay = RTC_WEEKDAY_MONDAY;
	rtc_date.Date = 14;
	rtc_date.Month = 9;
	rtc_date.Year = 20;

	HAL_RTC_SetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
	HAL_RTC_SetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);
	publishLogMessage("Wireless Com task started", wls_logs_box, reporter_t::Task_Wireless, LOG_TEXT_LEN);


	for( ;; )
	{
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
		osDelay(20);
	}
}

void IrrigationControlTask(void const *argument){

	irg_logs_box = osMailCreate(osMailQ(irg_logs_box), osThreadGetId());
	osEvent evt;
	plant_msg *msg = nullptr;

	RTC_TimeTypeDef rtc_time;
	RTC_DateTypeDef rtc_date;
	TimeStamp_t timestamp;

	bool sector_active_prev[SECTORS_AMOUNT] = {false, false, false, false};
	Scheduler sector_schedule[SECTORS_AMOUNT] = {Scheduler("SECTOR1"), Scheduler("SECTOR2"), Scheduler("SECTOR3"), Scheduler("SECTOR4")};
	uint8_t activities_cnt[SECTORS_AMOUNT]={0,0,0,0};
	uint8_t exceptions_cnt[SECTORS_AMOUNT]={0,0,0,0};
	uint16_t plants_cnt = 0;

	osDelay(1000);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	publishLogMessage("IrrgCtrl task started", irg_logs_box, reporter_t::Task_Irrigation, LOG_TEXT_LEN);

	ConcreteIrrigationSectorBuilder* sector_builder = new ConcreteIrrigationSectorBuilder; //leave as pointer to delete when not needed anymore
	sector_builder->producePlantWithDMAMoistureSensor("Pelargonia1", true)
					.producePlantWithDMAMoistureSensor("Kroton1", false)
					.producePlantWithDMAMoistureSensor("Kroton2", true)
					.producePlantWithDMAMoistureSensor("Truskawka1", true)
					.producePlantWithDMAMoistureSensor("Truskawka2", true)
					;

	//sector_builder->produceDRV8833PumpWithController(pump_controller_mode_t::external, 60, 300, pump1gpio, pump1led, pump1fault, pump1mode);
	std::unique_ptr<IrrigationSector>(p_sector1);
	p_sector1 = sector_builder->GetProduct();

	delete sector_builder;

	do{
		evt = osMailGet(plants_box, 1);
		if (evt.status == osEventMail){
			msg = (plant_msg*)evt.value.p;
			switch (msg->sector_nbr){
			case 0:
				//schedule[0].addException(msg->exception);
				break;
			case 1:
				//schedule[1].addException(msg->exception);
				break;
			case 2:
				//schedule[2].addException(msg->exception);
				break;
			case 3:
				//schedule[3].addException(msg->exception);
				break;
			default:
				break;
			}
		}
		osMailFree(plants_box, msg);
	}while(evt.status == osEventMail);



	for( ;; )
	{
		updateSectorsActivities(sector_schedule, activities_box);
		updateSectorsExceptions(sector_schedule, exceptions_box);

		HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);
		timestamp.day = rtc_date.Date;
		timestamp.hours = rtc_time.Hours;
		timestamp.minutes = rtc_time.Minutes;
		timestamp.month = rtc_date.Month;
		timestamp.seconds = rtc_time.Seconds;
		timestamp.weekday = rtc_date.WeekDay;
		timestamp.year = rtc_date.Year;

		for (uint8_t s_nbr=0; s_nbr<SECTORS_AMOUNT; ++s_nbr){
			if (sector_schedule[s_nbr].isAvailable()){
				if (sector_schedule[s_nbr].update(timestamp) != sector_active_prev[s_nbr]){
					if (sector_schedule[s_nbr].isActive()){
						HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
						publishLogMessage("Irrigation started", irg_logs_box, static_cast<reporter_t>(s_nbr), LOG_TEXT_LEN);
					}
					else{
						publishLogMessage("Irrigation finished", irg_logs_box, static_cast<reporter_t>(s_nbr), LOG_TEXT_LEN);
						HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
					}

					sector_active_prev[s_nbr] = sector_schedule[s_nbr].isActive();
				}
			activities_cnt[s_nbr] = sector_schedule[s_nbr].getActivitiesCount();
			exceptions_cnt[s_nbr] = sector_schedule[s_nbr].getExceptionsCount();
			}
		}

		//Placeholder for rest of the code

    	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
		osDelay(500);
	}

	//delete PlantWithDMAMoistureSensor1;
	//delete PlantWithDMAMoistureSensor2;
}




