
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
#include "usart.h"
#include <map>
#include <queue>
#include "HAL_UART_MsgBroker.h"
#include "MsgBrokerFactory.h"
#include "HAL_UART_ESP01S_MsgParser.h"


typedef std::pair<log_msg_prio_t, log_msg*> PAIR;
struct cmp_pair {
	bool operator()(const PAIR &a, const PAIR &b) {
		return a.first < b.first;
	};
};

extern TaskHandle_t xTaskToNotifyFromUsart2Rx;
extern TaskHandle_t xTaskToNotifyFromUsart2Tx;
extern const UBaseType_t xArrayIndex;



osThreadId SysMonitorTaskHandle;
osThreadId SDCardTaskHandle;
osThreadId WirelessCommTaskHandle;
osThreadId IrrigationControlTaskHandle;

osMailQId activities_box;
osMailQDef(activities_box, 42, activity_msg);
osMailQId exceptions_box;
osMailQDef(exceptions_box, 6, exception_msg);
osMailQId plants_config_box;
osMailQDef(plants_config_box, 20, plant_config_msg);
osMailQId sys_logs_box;
osMailQDef(sys_logs_box, 10, log_msg);
osMailQId irg_logs_box;
osMailQDef(irg_logs_box, 10, log_msg);
osMailQId wls_logs_box;
osMailQDef(wls_logs_box, 10, log_msg);

osSemaphoreId time_rdy_sem;
osSemaphoreDef(time_rdy_sem);
osSemaphoreId config_rdy_sem;
osSemaphoreDef(config_rdy_sem);


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

/* USER CODE END GET_TIMER_TASK_MEMORY */

uint8_t publishLogMessage(std::string_view msg_txt, osMailQId &mail_box, const reporter_t &_reporter, const log_msg_prio_t &_msg_priority)
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
	msg->priority = _msg_priority;
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

double calculateTimeDiff(const TimeStamp_t& _past, const TimeStamp_t& _now) {
	struct tm tm_timestamp_now { _now.seconds, _now.minutes, _now.hours, _now.day, _now.month - 1, _now.year + 100 };
	struct tm tm_timestamp_past { _past.seconds, _past.minutes, _past.hours, _past.day, _past.month - 1, _past.year + 100 };
	time_t now = mktime(&tm_timestamp_now);
	time_t past = mktime(&tm_timestamp_past);

	using milliseconds = std::chrono::milliseconds;

	auto tp_now = std::chrono::system_clock::from_time_t(now);
	tp_now += milliseconds(_now.milliseconds);
	auto tp_past = std::chrono::system_clock::from_time_t(past);
	tp_past += milliseconds(_past.milliseconds);
	std::chrono::duration<double, std::milli>  dt_ms = tp_now - tp_past;

	return dt_ms.count() /1000.0;
}

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

  osThreadDef(irrigationTask, IrrigationControlTask, osPriorityNormal, 0, 60*configMINIMAL_STACK_SIZE);
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

	publishLogMessage("Sys monitor task started", sys_logs_box, reporter_t::Task_SysMonitor, log_msg_prio_t::INFO);

	for(;;)
	{
    	min_heap_size = xPortGetMinimumEverFreeHeapSize();
    	if (min_heap_size != prev_min_heap_size){
    		prev_min_heap_size = min_heap_size;
    		std::string_view msg = "Min HEAP size:" + patch::to_string(min_heap_size) + "b";
    		publishLogMessage(msg, sys_logs_box, reporter_t::Task_SysMonitor, log_msg_prio_t::HIGH);
    	}
    	vTaskGetInfo(SDCardTaskHandle, &TaskStatus, FreeStackSpace, State);
    	if (TaskStatus.usStackHighWaterMark != SDCardPrevStackHighWaterMark){
    		std::string_view msg =  "SDCardTask HWMark:" + patch::to_string(TaskStatus.usStackHighWaterMark * 2) + "b";
    		SDCardPrevStackHighWaterMark = TaskStatus.usStackHighWaterMark;
    		publishLogMessage(msg, sys_logs_box, reporter_t::Task_SysMonitor, log_msg_prio_t::HIGH);
    	}
    	vTaskGetInfo(WirelessCommTaskHandle, &TaskStatus, FreeStackSpace, State);
    	if (TaskStatus.usStackHighWaterMark != WirelessCommPrevStackHighWaterMark){
    		std::string_view msg =  "WlsCom HWMark:"+ patch::to_string(TaskStatus.usStackHighWaterMark * 2) + "b";
    		WirelessCommPrevStackHighWaterMark = TaskStatus.usStackHighWaterMark;
    		publishLogMessage(msg, sys_logs_box, reporter_t::Task_SysMonitor, log_msg_prio_t::HIGH);
    	}
    	vTaskGetInfo(IrrigationControlTaskHandle, &TaskStatus, FreeStackSpace, State);
    	if (TaskStatus.usStackHighWaterMark != IrrigationControlPrevStackHighWaterMark){
    		std::string_view msg =  "IrrCtrl HWMark:"+ patch::to_string(TaskStatus.usStackHighWaterMark * 2) + "b";
    		IrrigationControlPrevStackHighWaterMark = TaskStatus.usStackHighWaterMark;
    		publishLogMessage(msg, sys_logs_box, reporter_t::Task_SysMonitor, log_msg_prio_t::HIGH);
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
	config_rdy_sem = osSemaphoreCreate(osSemaphore(config_rdy_sem), 1);

	const std::array<std::string_view, 4> config_file_candidates = {"SECTOR1.TXT", "SECTOR2.TXT", "SECTOR3.TXT",  "SECTOR4.TXT"};
	bool mount_success = false;
	activity_msg *activity = nullptr;
	activities_box = osMailCreate(osMailQ(activities_box), osThreadGetId());
	exception_msg *exception = nullptr;
	exceptions_box = osMailCreate(osMailQ(exceptions_box), osThreadGetId());
	plant_config_msg *plant = nullptr;
	plants_config_box = osMailCreate(osMailQ(plants_config_box), osThreadGetId());
	osEvent evt;
	log_msg *sys_message = nullptr;
	log_msg *irg_message = nullptr;
	log_msg *wls_message = nullptr;

	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
	osSemaphoreWait(config_rdy_sem, osWaitForever);

	if(f_mount(&file_system, logical_drive, 1) == FR_OK){
		mount_success = true;
		logger.writeLog("SDCard mount successful!", &log_file, reporter_t::Task_SDCard);

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
							char config_line[MAXIMUM(ACTIVITY_LENGTH+1, EXCEPTION_LENGTH+1)] = "";
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
								else if(config_line[0] == 'P'){	//format P001:Y,1,Pelargonia...
									plant = (plant_config_msg*)osMailAlloc(plants_config_box, osWaitForever);
									plant->sector_nbr = sector_nbr;
									const std::string str(config_line);
									plant->rain_exposed = str.substr(5,1) == "Y" ? true : false;
									plant->type = atoi(str.substr(7,1).c_str());
									str.copy(plant->name, MINIMUM(str.length() - 10, PLANT_NAME_LEN - 2), 9);
									while (osMailPut(plants_config_box, plant) != osOK);
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

	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	osSemaphoreRelease(config_rdy_sem);

    for( ;; )
    {
		if(mount_success){
			std::multimap<log_msg_prio_t, log_msg*> msg_map = {};
			do{
				evt = osMailGet(sys_logs_box, 1);
				if (evt.status == osEventMail){
					sys_message = (log_msg*)evt.value.p;
					msg_map.insert({ sys_message->priority, sys_message });
				}
				osMailFree(sys_logs_box, sys_message);
			}while(evt.status == osEventMail);

			do{
				evt = osMailGet(irg_logs_box, 1);
				if (evt.status == osEventMail){
					irg_message = (log_msg*)evt.value.p;
					msg_map.insert({ irg_message->priority, irg_message });
				}
				osMailFree(irg_logs_box, irg_message);
			}while(evt.status == osEventMail);

			do{
				evt = osMailGet(wls_logs_box, 1);
				if (evt.status == osEventMail){
					wls_message = (log_msg*)evt.value.p;
					msg_map.insert({ wls_message->priority, wls_message });
				}
				osMailFree(wls_logs_box, wls_message);
			}while(evt.status == osEventMail);

			std::priority_queue<PAIR, std::vector<PAIR>, cmp_pair> logs_queue{};
			for (const auto &msg: msg_map ) {
				logs_queue.push(msg);
				}
			while (!logs_queue.empty()) {
				PAIR top = logs_queue.top() ;
				logger.writeLog(top.second, &log_file);
				logs_queue.pop();
				}
		}

    	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
    	osDelay(200);
    }
}


void WirelessCommTask(void const *argument)
{
	wls_logs_box = osMailCreate(osMailQ(wls_logs_box), osThreadGetId());
	time_rdy_sem = osSemaphoreCreate(osSemaphore(time_rdy_sem), 1);

	publishLogMessage("Wireless Comm task started", wls_logs_box, reporter_t::Task_Wireless, log_msg_prio_t::INFO);
	publishLogMessage("Updating current time...", wls_logs_box, reporter_t::Task_Wireless, log_msg_prio_t::INFO);

	osSemaphoreWait(time_rdy_sem, osWaitForever);

	 /* Store the handle of the calling task. */
    xTaskToNotifyFromUsart2Rx = xTaskGetCurrentTaskHandle();
	xTaskToNotifyFromUsart2Tx = xTaskGetCurrentTaskHandle();

	MsgBrokerPtr p_broker;
	p_broker = MsgBrokerFactory::create(msg_broker_type_t::hal_uart, &huart2);
	HAL_UART_ESP01S_MsgParser esp01s_parser1;
	p_broker->setParser(&esp01s_parser1);

	if (!p_broker->requestData(recipient_t::ntp_server, "CurrentTime", true)){
		publishLogMessage("Curr time transmit request failed!", wls_logs_box, reporter_t::Task_Wireless, log_msg_prio_t::CRITICAL);
	}

	p_broker->readData(EXT_TIME_STR_LEN);
	//p_broker->publishData(recipient_t::google_home, "Pelargonia", { { "Soil moisture", 67}, { "is exposed", 0 } });
	//xTaskToNotifyFromUsart2Tx = NULL;

	publishLogMessage("Current time set!", wls_logs_box, reporter_t::Task_Wireless, log_msg_prio_t::CRITICAL);
	osSemaphoreRelease(time_rdy_sem);


	for( ;; )
	{
		if(!p_broker->sendMsg(recipient_t::google_home, "I'm alive!")){
			publishLogMessage("I'm alive! transmit failed!", wls_logs_box, reporter_t::Task_Wireless, log_msg_prio_t::CRITICAL);
		}
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
		osDelay(200);
	}
}

void IrrigationControlTask(void const *argument){

	irg_logs_box = osMailCreate(osMailQ(irg_logs_box), osThreadGetId());
	osEvent evt;
	plant_config_msg *msg = nullptr;

	RTC_TimeTypeDef rtc_time;
	RTC_DateTypeDef rtc_date;
	TimeStamp_t timestamp, timestamp_prev;

	bool sector_active_prev[SECTORS_AMOUNT] = {};
	Scheduler sector_schedule[SECTORS_AMOUNT] = {Scheduler("SECTOR1"), Scheduler("SECTOR2"), Scheduler("SECTOR3"), Scheduler("SECTOR4")};
	std::array<std::array<struct gpio_s, 2>, SECTORS_AMOUNT> pump_ctrl_gpio;
	pump_ctrl_gpio[0] = {{{ GPIOE, GPIO_PIN_0 }, { GPIOE, GPIO_PIN_1 }}};
	pump_ctrl_gpio[1] = {{{ GPIOA, GPIO_PIN_15 }, { GPIOA, GPIO_PIN_14 }}};
	pump_ctrl_gpio[2] = {{{ GPIOE, GPIO_PIN_15 }, { GPIOE, GPIO_PIN_14 }}};
	pump_ctrl_gpio[3] = {{{ GPIOE, GPIO_PIN_10 }, { GPIOE, GPIO_PIN_9 }}};
	const struct gpio_s pump_led_gpio[SECTORS_AMOUNT] = {{GPIOE, GPIO_PIN_2},{GPIOA, GPIO_PIN_13},{GPIOE, GPIO_PIN_13},{GPIOE, GPIO_PIN_8}};
	const struct gpio_s pump_fault_gpio[SECTORS_AMOUNT] = {{GPIOE, GPIO_PIN_3},{GPIOA, GPIO_PIN_10},{GPIOE, GPIO_PIN_12},{GPIOE, GPIO_PIN_7}};
	const struct gpio_s pump_mode_gpio[SECTORS_AMOUNT] = {{GPIOE, GPIO_PIN_4},{GPIOA, GPIO_PIN_9},{GPIOE, GPIO_PIN_11},{GPIOB, GPIO_PIN_2}};
	const struct gpio_s opt_wl_sensor1_gpio = {GPIOB, GPIO_PIN_12};
	std::unique_ptr<IrrigationSector>(p_sector[SECTORS_AMOUNT]);
	std::unique_ptr<Watertank>(p_watertank);
	const uint32_t refresh_interval_msec = 500.0;
	double dt_seconds = 500.0_msec;
	bool time_set{false};

	osSemaphoreWait(time_rdy_sem, osWaitForever);
	osSemaphoreWait(config_rdy_sem, osWaitForever);

	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	publishLogMessage("IrrgCtrl task started", irg_logs_box, reporter_t::Task_Irrigation, log_msg_prio_t::INFO);

	ConcreteIrrigationSectorBuilder* sector_builder = new ConcreteIrrigationSectorBuilder[SECTORS_AMOUNT]; //leave as pointer to delete when not needed anymore
	for (uint8_t i=0; i<SECTORS_AMOUNT; ++i){
		sector_builder[i].produceDRV8833PumpWithController(pump_controller_mode_t::external, PUMP_IDLETIME_REQUIRED_SEC, PUMP_RUNTIME_LIMIT_SEC, pump_ctrl_gpio[i], pump_led_gpio[i], pump_fault_gpio[i], pump_mode_gpio[i]);
	}

	do{
		evt = osMailGet(plants_config_box, 10);
		if (evt.status == osEventMail){
			msg = (plant_config_msg*)evt.value.p;
			if (msg->type == 1){
				sector_builder[msg->sector_nbr].producePlantWithDMAMoistureSensor(msg->name, msg->rain_exposed);
				std::string_view text = "S" + patch::to_string(static_cast<reporter_t>(msg->sector_nbr)) + " got plant: " + msg->name;
				publishLogMessage(text, irg_logs_box, reporter_t::Task_Irrigation, log_msg_prio_t::LOW);
			}
		}
		osMailFree(plants_config_box, msg);
	}while(evt.status == osEventMail);

	for (uint8_t i=0; i<SECTORS_AMOUNT; ++i){
		 p_sector[i] = sector_builder[i].GetProduct();
		 p_sector[i]->setWateringState(false);
	}


	delete[] sector_builder;

	ConcreteWatertankBuilder *watertank_builder = new ConcreteWatertankBuilder;
	watertank_builder->produceOpticalWaterLevelSensor(0.0825_m,  opt_wl_sensor1_gpio);
	p_watertank = watertank_builder->GetProduct();

	p_watertank->setTankStateHysteresis(4.0_sec, 4.0_sec);
	p_watertank->setHeight(55.0_cm);
	p_watertank->setVolume(50.0_l);

	delete watertank_builder;

	for( ;; )
	{
		updateSectorsActivities(sector_schedule, activities_box);
		updateSectorsExceptions(sector_schedule, exceptions_box);

		if (HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN) == HAL_OK && HAL_RTC_GetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN) == HAL_OK){
			timestamp = {rtc_date.WeekDay, rtc_time.Hours, rtc_time.Minutes, rtc_time.Seconds, rtc_date.Date, rtc_date.Month, rtc_date.Year, 1000 * (rtc_time.SecondFraction - rtc_time.SubSeconds) / (rtc_time.SecondFraction + 1)};
			time_set == true ? dt_seconds = calculateTimeDiff(timestamp_prev, timestamp) : time_set = true;
			timestamp_prev = {timestamp};
		}

		//TODO:publishLogMessage pass as pointer and invoke only once per state change?
		if(true/*p_watertank->update(dt_seconds)*/){ //TODO: base decision on tank's state when connected phisically
			for (uint8_t s_nbr=0; s_nbr<SECTORS_AMOUNT; ++s_nbr){
				if (sector_schedule[s_nbr].isAvailable()){
					if (sector_schedule[s_nbr].update(timestamp) != sector_active_prev[s_nbr]){
						if (sector_schedule[s_nbr].isActive()){
							publishLogMessage("Irrigation start", irg_logs_box, static_cast<reporter_t>(s_nbr), log_msg_prio_t::MEDIUM);
							HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
							p_sector[s_nbr]->setWateringState(true);
						}
						else{
							publishLogMessage("Irrigation stop", irg_logs_box, static_cast<reporter_t>(s_nbr), log_msg_prio_t::MEDIUM);
							HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
							p_sector[s_nbr]->setWateringState(false);
						}
						sector_active_prev[s_nbr] = sector_schedule[s_nbr].isActive();
					}
				}
			}
		}
		else{
			publishLogMessage("Irrigation stop, tank error", irg_logs_box, reporter_t::Watertank1, log_msg_prio_t::CRITICAL);
			for (uint8_t s_nbr=0; s_nbr<SECTORS_AMOUNT; ++s_nbr){
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
				p_sector[s_nbr]->setWateringState(false);
			}
		}

		for (uint8_t s_nbr=0; s_nbr<SECTORS_AMOUNT; ++s_nbr){
			p_sector[s_nbr]->update(dt_seconds);
		}

    	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
		osDelay(refresh_interval_msec);
	}

}




