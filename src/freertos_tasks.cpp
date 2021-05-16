
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
#include <chrono>
#include "HAL_UART_DMA_MsgBroker.h"
#include "MsgBrokerFactory.h"
#include "Esp01s_MsgParser.h"
#include "Messages.h"
#include "MsgBroker.h"
#include "MsgBrokerFactory.h"
#include "MsgEncoder.h"
#include "SMS_MsgParser.h"

extern TaskHandle_t xTaskToNotifyFromUsart2Rx;
extern TaskHandle_t xTaskToNotifyFromUsart2Tx;
extern TaskHandle_t xTaskToNotifyFromUsart3Rx;
extern TaskHandle_t xTaskToNotifyFromUsart3Tx;
extern const UBaseType_t xArrayIndex;

TaskHandle_t xSDCardNotifyHandle = NULL;
TaskHandle_t xSysMonNotifyHandle = NULL;
TaskHandle_t xIrgCtrlNotifyHandle = NULL;
TaskHandle_t xSmsRxNotifyHandle = NULL;
TaskHandle_t xSmsTxNotifyHandle = NULL;
TaskHandle_t xWlsTxNotifyHandle = NULL;
const UBaseType_t xTaskCount = 1;


osThreadId SysMonitorTaskHandle;
osThreadId SDCardTaskHandle;
osThreadId WirelessReceiverTaskHandle;
osThreadId WirelessTransmitterTaskHandle;
osThreadId IrrigationControlTaskHandle;
osThreadId SmsReceiverTaskHandle;
osThreadId SmsTransmitterTaskHandle;

osMailQId activities_box;
osMailQDef(activities_box, 42, ActivityMsg);
osMailQId exceptions_box;
osMailQDef(exceptions_box, 6, ExceptionMsg);
osMailQId plants_config_box;
osMailQDef(plants_config_box, 20, PlantConfigMsg);
osMailQId sys_logs_box;
osMailQDef(sys_logs_box, 10, LogMsg);
osMailQId irg_logs_box;
osMailQDef(irg_logs_box, 10, LogMsg);
osMailQId wls_rx_logs_box;
osMailQDef(wls_rx_logs_box, 10, LogMsg);
osMailQId wls_tx_logs_box;
osMailQDef(wls_tx_logs_box, 10, LogMsg);
osMailQId sms_rx_logs_box;
osMailQDef(sms_rx_logs_box, 10, LogMsg);
osMailQId sms_tx_logs_box;
osMailQDef(sms_tx_logs_box, 10, LogMsg);
osMailQId internal_msg_box;
osMailQDef(internal_msg_box, 10, InternalMessage);

using namespace std::chrono;

void SysMonitorTask(void const * argument);
void SDCardTask(void const *argument);
void WirelessReceiverTask(void const *argument);
void WirelessTransmitterTask(void const *argument);
void IrrigationControlTask(void const *argument);
void SmsReceiverTask(void const *argument);
void SmsTransmitterTask(void const *argument);

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

osEvent updateSectorsActivities(Scheduler *schedule, osMailQId &mail_box)
{
	osEvent evt;
	ActivityMsg *msg = nullptr;

	do{
		evt = osMailGet(mail_box, 1);
		if (evt.status == osEventMail){
			msg = (ActivityMsg*)evt.value.p;
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
	ExceptionMsg *msg = nullptr;

	do{
		evt = osMailGet(mail_box, 1);
		if (evt.status == osEventMail){
			msg = (ExceptionMsg*)evt.value.p;
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

  //WirelessReceiverTask must start first since it gets current time from external server
  osThreadDef(wirelessreceiverTask, WirelessReceiverTask, osPriority::osPriorityHigh, 0, 20*configMINIMAL_STACK_SIZE);
  WirelessReceiverTaskHandle = osThreadCreate(osThread(wirelessreceiverTask), NULL);
  
  osThreadDef(sysmonitorTask, SysMonitorTask, osPriority::osPriorityRealtime, 0, 30*configMINIMAL_STACK_SIZE);
  SysMonitorTaskHandle = osThreadCreate(osThread(sysmonitorTask), NULL);

  osThreadDef(sdcardTask, SDCardTask, osPriority::osPriorityNormal, 0, 40*configMINIMAL_STACK_SIZE);
  SDCardTaskHandle = osThreadCreate(osThread(sdcardTask), NULL);
  
  osThreadDef(irrigationTask, IrrigationControlTask, osPriority::osPriorityNormal, 0,20*configMINIMAL_STACK_SIZE);
  IrrigationControlTaskHandle = osThreadCreate(osThread(irrigationTask), NULL);
  
  osThreadDef(wirelesstransmitterTask, WirelessTransmitterTask, osPriority::osPriorityNormal, 0, 20*configMINIMAL_STACK_SIZE);
  WirelessTransmitterTaskHandle = osThreadCreate(osThread(wirelesstransmitterTask), NULL);
  
  osThreadDef(smsreceiverTask, SmsReceiverTask, osPriority::osPriorityNormal, 0, 20*configMINIMAL_STACK_SIZE);
  SmsReceiverTaskHandle = osThreadCreate(osThread(smsreceiverTask), NULL);
  
  osThreadDef(smstransmitterTask, SmsTransmitterTask, osPriority::osPriorityNormal, 0, 20*configMINIMAL_STACK_SIZE);
  SmsTransmitterTaskHandle = osThreadCreate(osThread(smstransmitterTask), NULL);

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
	xSysMonNotifyHandle = xTaskGetCurrentTaskHandle();
	size_t min_heap_size = 0;
	size_t prev_min_heap_size = 0;
	TaskStatus_t TaskStatus;
	configSTACK_DEPTH_TYPE SDCardPrevStackHighWaterMark = 0;
	configSTACK_DEPTH_TYPE WirelessRxPrevStackHighWaterMark = 0;
	configSTACK_DEPTH_TYPE WirelessTxPrevStackHighWaterMark = 0;
	configSTACK_DEPTH_TYPE IrrigationControlPrevStackHighWaterMark = 0;
	configSTACK_DEPTH_TYPE SmsRxPrevStackHighWaterMark = 0;
	configSTACK_DEPTH_TYPE SmsTxPrevStackHighWaterMark = 0;
	BaseType_t FreeStackSpace = 1;
	eTaskState State = eInvalid;
	sys_logs_box = osMailCreate(osMailQ(sys_logs_box), osThreadGetId());
	const uint32_t refresh_interval_msec = 2000;

	ulTaskNotifyTake( xTaskCount, osWaitForever);

	HAL_FatFs_Logger::publishLogMessage("Running", sys_logs_box, reporter_t::Task_SysMonitor, log_msg_prio_t::INFO);

	for(;;)
	{
    	min_heap_size = xPortGetMinimumEverFreeHeapSize();
    	if (min_heap_size != prev_min_heap_size){
    		prev_min_heap_size = min_heap_size;
    		std::string_view msg = "Min HEAP size:" + patch::to_string(min_heap_size) + "b";
    		HAL_FatFs_Logger::publishLogMessage(msg, sys_logs_box, reporter_t::Task_SysMonitor, log_msg_prio_t::HIGH);
    	}
    	vTaskGetInfo(SDCardTaskHandle, &TaskStatus, FreeStackSpace, State);
    	if (TaskStatus.usStackHighWaterMark != SDCardPrevStackHighWaterMark){
    		std::string_view msg =  "SDCardTask HWMark:" + patch::to_string(TaskStatus.usStackHighWaterMark * 2) + "b";
    		SDCardPrevStackHighWaterMark = TaskStatus.usStackHighWaterMark;
    		HAL_FatFs_Logger::publishLogMessage(msg, sys_logs_box, reporter_t::Task_SysMonitor, log_msg_prio_t::HIGH);
    	}
    	vTaskGetInfo(WirelessReceiverTaskHandle, &TaskStatus, FreeStackSpace, State);
    	if (TaskStatus.usStackHighWaterMark != WirelessRxPrevStackHighWaterMark){
    		std::string_view msg =  "WlsRx HWMark:"+ patch::to_string(TaskStatus.usStackHighWaterMark * 2) + "b";
    		WirelessRxPrevStackHighWaterMark = TaskStatus.usStackHighWaterMark;
    		HAL_FatFs_Logger::publishLogMessage(msg, sys_logs_box, reporter_t::Task_SysMonitor, log_msg_prio_t::HIGH);
    	}
		vTaskGetInfo(WirelessTransmitterTaskHandle, &TaskStatus, FreeStackSpace, State);
    	if (TaskStatus.usStackHighWaterMark != WirelessTxPrevStackHighWaterMark){
    		std::string_view msg =  "WlsTx HWMark:"+ patch::to_string(TaskStatus.usStackHighWaterMark * 2) + "b";
    		WirelessTxPrevStackHighWaterMark = TaskStatus.usStackHighWaterMark;
    		HAL_FatFs_Logger::publishLogMessage(msg, sys_logs_box, reporter_t::Task_SysMonitor, log_msg_prio_t::HIGH);
    	}
    	vTaskGetInfo(IrrigationControlTaskHandle, &TaskStatus, FreeStackSpace, State);
    	if (TaskStatus.usStackHighWaterMark != IrrigationControlPrevStackHighWaterMark){
    		std::string_view msg =  "IrrCtrl HWMark:"+ patch::to_string(TaskStatus.usStackHighWaterMark * 2) + "b";
    		IrrigationControlPrevStackHighWaterMark = TaskStatus.usStackHighWaterMark;
    		HAL_FatFs_Logger::publishLogMessage(msg, sys_logs_box, reporter_t::Task_SysMonitor, log_msg_prio_t::HIGH);
    	}
		vTaskGetInfo(SmsReceiverTaskHandle, &TaskStatus, FreeStackSpace, State);
    	if (TaskStatus.usStackHighWaterMark != SmsRxPrevStackHighWaterMark){
    		std::string_view msg =  "SmsRx HWMark:"+ patch::to_string(TaskStatus.usStackHighWaterMark * 2) + "b";
    		SmsRxPrevStackHighWaterMark = TaskStatus.usStackHighWaterMark;
    		HAL_FatFs_Logger::publishLogMessage(msg, sys_logs_box, reporter_t::Task_SysMonitor, log_msg_prio_t::HIGH);
    	}
    	if (TaskStatus.usStackHighWaterMark != SmsTxPrevStackHighWaterMark){
    		std::string_view msg =  "SmsTx HWMark:"+ patch::to_string(TaskStatus.usStackHighWaterMark * 2) + "b";
    		SmsTxPrevStackHighWaterMark = TaskStatus.usStackHighWaterMark;
    		HAL_FatFs_Logger::publishLogMessage(msg, sys_logs_box, reporter_t::Task_SysMonitor, log_msg_prio_t::HIGH);
    	}
		osDelay(refresh_interval_msec);
	}
}

void SDCardTask(void const *argument)
{
	xSDCardNotifyHandle = xTaskGetCurrentTaskHandle();

    FIL log_file;
	FIL config_file;
	char logical_drive[4] = {0, 0, 0, 0};   /* SD logical drive path */
	FATFS file_system;    /* File system object for SD logical drive */
	DIR directory;
	FILINFO file_info;
	char cwd_buffer[80] = "/";

	ActivityMsg *activity = nullptr;
	activities_box = osMailCreate(osMailQ(activities_box), osThreadGetId());
	ExceptionMsg *exception = nullptr;
	exceptions_box = osMailCreate(osMailQ(exceptions_box), osThreadGetId());
	PlantConfigMsg *plant = nullptr;
	plants_config_box = osMailCreate(osMailQ(plants_config_box), osThreadGetId());
	const uint32_t refresh_interval_msec = 200;

	const std::array<std::string_view, 4> config_file_candidates = {"SECTOR1.TXT", "SECTOR2.TXT", "SECTOR3.TXT",  "SECTOR4.TXT"};
	HAL_FatFs_Logger &logger = HAL_FatFs_Logger::createInstance();
	bool mount_success = false;

	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
	ulTaskNotifyTake( xTaskCount, osWaitForever);

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
									activity = (ActivityMsg*)osMailAlloc(activities_box, osWaitForever);
									if (activity != NULL){
										activity->sector_nbr = sector_nbr;
										activity->activity = Scheduler::parseActivity(config_line);
										while (osMailPut(activities_box, activity) != osOK);
									}
								}
								else if(config_line[0] == 'E'){
									exception = (ExceptionMsg*)osMailAlloc(exceptions_box, osWaitForever);
									if (exception != NULL){
										exception->sector_nbr = sector_nbr;
										exception->exception = Scheduler::parseException(config_line);
										while (osMailPut(exceptions_box, exception) != osOK);
									}
								}
								else if(config_line[0] == 'P'){	//format P001:Y,1,Pelargonia...
									plant = (PlantConfigMsg*)osMailAlloc(plants_config_box, osWaitForever);
									if (plant != NULL){
										plant->sector_nbr = sector_nbr;
										const std::string str(config_line);
										plant->rain_exposed = str.substr(5,1) == "Y" ? true : false;
										plant->type = atoi(str.substr(7,1).c_str());
										str.copy(plant->name, MINIMUM(str.length() - 10, PLANT_NAME_LEN - 2), 9);
										while (osMailPut(plants_config_box, plant) != osOK);
									}
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
	while( xIrgCtrlNotifyHandle == NULL );
    xTaskNotifyGive( xIrgCtrlNotifyHandle);

    for( ;; )
    {
		if(mount_success){
			logger.accumulateLogs(sys_logs_box);
			logger.accumulateLogs(irg_logs_box);
			logger.accumulateLogs(wls_rx_logs_box);
			logger.accumulateLogs(wls_tx_logs_box);
			logger.accumulateLogs(sms_rx_logs_box);
			logger.accumulateLogs(sms_tx_logs_box);
			logger.releaseLogsToFile(&log_file);
		}

    	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
    	osDelay(refresh_interval_msec);
    }
}

void WirelessReceiverTask(void const *argument)
{
	 /* Store the handle of the calling task. */
    xTaskToNotifyFromUsart2Rx = xTaskGetCurrentTaskHandle();
	wls_rx_logs_box = osMailCreate(osMailQ(wls_rx_logs_box), osThreadGetId());

	MsgBrokerPtr esp01s_msg_receiver = MsgBrokerFactory::create(MsgBrokerType::hal_uart_dma, esp01s_msg_len, &huart2);
	Esp01s_MsgParser esp01s_msg_parser;
	esp01s_msg_receiver->setDefaultParser(&esp01s_msg_parser);
	esp01s_msg_receiver->setInternalAddresses(&internal_entities);

	esp01s_msg_receiver->readData();
	IncomingMessage msg = esp01s_msg_receiver->getIncoming();
	if(msg.sender.object == ExternalObject_t::ntp_server && msg.recipient.object == InternalObject_t::system){
		setRtcFromIncoming(msg);
		HAL_FatFs_Logger::publishLogMessage("Current time updated!", wls_rx_logs_box, reporter_t::Task_WirelessReceiver, log_msg_prio_t::CRITICAL);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
	}

    while( xSDCardNotifyHandle == NULL || xSysMonNotifyHandle == NULL  || xSmsRxNotifyHandle == NULL || xSmsTxNotifyHandle == NULL || xWlsTxNotifyHandle == NULL);
    xTaskNotifyGive( xSDCardNotifyHandle);
	xTaskNotifyGive( xSysMonNotifyHandle);
	xTaskNotifyGive( xSmsRxNotifyHandle);
	xTaskNotifyGive( xSmsTxNotifyHandle);
	xTaskNotifyGive( xWlsTxNotifyHandle);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);

	HAL_FatFs_Logger::publishLogMessage("Running", wls_rx_logs_box, reporter_t::Task_WirelessReceiver, log_msg_prio_t::INFO);

	for( ;; )
	{
		osDelay(1000);
		//if (esp01s_msg_receiver->readData()){
		//	IncomingMessage msg = esp01s_msg_receiver->getIncoming();
		//}
	}
}

void WirelessTransmitterTask(void const *argument)
{
	xWlsTxNotifyHandle = xTaskGetCurrentTaskHandle();
	xTaskToNotifyFromUsart2Tx = xTaskGetCurrentTaskHandle();
	wls_tx_logs_box = osMailCreate(osMailQ(wls_rx_logs_box), osThreadGetId());
	constexpr uint32_t refresh_interval_msec = 500;

	MsgBrokerPtr esp01s_msg_transmitter = MsgBrokerFactory::create(MsgBrokerType::hal_uart_dma, esp01s_msg_len, &huart2);
	JsonPubEncoder json_pub_serializer{};
	JsonReqEncoder json_get_serializer{};
	JsonMsgEncoder json_msg_serializer{};

	esp01s_msg_transmitter->setDefaultEncoder(&json_pub_serializer);
	esp01s_msg_transmitter->setInternalAddresses(&internal_entities);
	esp01s_msg_transmitter->setExternalAddresses(&esp01s_external_addresses);
	esp01s_msg_transmitter->sendMsg({ ExternalObject_t::google_home, 1 }, { InternalObject_t::sector, 0 }, "I'm alive", true, &json_msg_serializer);

	ulTaskNotifyTake( xTaskCount, osWaitForever);
	HAL_FatFs_Logger::publishLogMessage("Running", wls_tx_logs_box, reporter_t::Task_WirelessTransmitter, log_msg_prio_t::INFO);


	for( ;; )
	{
		//TODO: collect internal msg and pass to transmitter
		esp01s_msg_transmitter->sendMsg({ ExternalObject_t::google_home, 1 }, { InternalObject_t::sector, 0 }, "I'm alive1", true, &json_msg_serializer);
		osDelay(refresh_interval_msec);
	}
}

void SmsReceiverTask(void const *argument)
{
	xSmsRxNotifyHandle = xTaskGetCurrentTaskHandle();
    xTaskToNotifyFromUsart3Rx = xTaskGetCurrentTaskHandle();
	const uint32_t refresh_interval_msec = 100;
	sms_rx_logs_box = osMailCreate(osMailQ(sms_rx_logs_box), osThreadGetId());

	ulTaskNotifyTake( xTaskCount, osWaitForever);
	HAL_FatFs_Logger::publishLogMessage("Running", sms_rx_logs_box, reporter_t::Task_SmsReceiver, log_msg_prio_t::INFO);

	for( ;; )
	{
		//event-based
		//HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
		osDelay(refresh_interval_msec);
	}
}

void SmsTransmitterTask(void const *argument)
{
	constexpr uint32_t refresh_interval_msec = 1000;
	xSmsTxNotifyHandle = xTaskGetCurrentTaskHandle();
	 /* Store the handle of the calling task. */
	xTaskToNotifyFromUsart3Tx = xTaskGetCurrentTaskHandle();
	sms_tx_logs_box = osMailCreate(osMailQ(sms_tx_logs_box), osThreadGetId());

	ulTaskNotifyTake( xTaskCount, osWaitForever);
	HAL_FatFs_Logger::publishLogMessage("Running", sms_tx_logs_box, reporter_t::Task_SmsTransmitter, log_msg_prio_t::INFO);

	for( ;; )
	{
		//TODO: collect internal msgs and send
		//HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
		osDelay(refresh_interval_msec);
	}
}


void IrrigationControlTask(void const *argument)
{
	xIrgCtrlNotifyHandle = xTaskGetCurrentTaskHandle();
	irg_logs_box = osMailCreate(osMailQ(irg_logs_box), osThreadGetId());

	RTC_TimeTypeDef rtc_time{};
	RTC_DateTypeDef rtc_date{};
	TimeStamp_t timestamp{}, timestamp_prev{};

	bool sector_active_prev[SECTORS_AMOUNT]{};
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
	const uint32_t refresh_interval_msec = 500;
	double dt_seconds = 500.0_msec;
	bool time_set{false};

	ulTaskNotifyTake( xTaskCount, osWaitForever);
	HAL_FatFs_Logger::publishLogMessage("Running", irg_logs_box, reporter_t::Task_Irrigation, log_msg_prio_t::INFO);

	//TODO: use director class for building sectors
	ConcreteIrrigationSectorBuilder* sector_builder = new ConcreteIrrigationSectorBuilder[SECTORS_AMOUNT]; //leave as pointer to delete when not needed anymore
	for (uint8_t i=0; i<SECTORS_AMOUNT; ++i){
		sector_builder[i].produceDRV8833PumpWithController(pump_controller_mode_t::external, PUMP_IDLETIME_REQUIRED_SEC, PUMP_RUNTIME_LIMIT_SEC, pump_ctrl_gpio[i], pump_led_gpio[i], pump_fault_gpio[i], pump_mode_gpio[i]);
	}
	
	//TODO: looks like a function
	osEvent evt{};
	PlantConfigMsg *msg{nullptr};
	do{
		evt = osMailGet(plants_config_box, 10);
		if (evt.status == osEventMail){
			msg = (PlantConfigMsg*)evt.value.p;
			if (msg->type == 1){
				sector_builder[msg->sector_nbr].producePlantWithDMAMoistureSensor(msg->name, msg->rain_exposed);
				std::string_view text = "S" + patch::to_string(static_cast<reporter_t>(msg->sector_nbr) + 1) + " got plant: " + msg->name;
				HAL_FatFs_Logger::publishLogMessage(text, irg_logs_box, reporter_t::Task_Irrigation, log_msg_prio_t::LOW);
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

		//TODO:HAL_FatFs_Logger::publishLogMessage pass as pointer and invoke only once per state change?
		if(true/*p_watertank->update(dt_seconds)*/){ //TODO: base decision on tank's state when connected phisically
			for (uint8_t s_nbr=0; s_nbr<SECTORS_AMOUNT; ++s_nbr){
				if (sector_schedule[s_nbr].isAvailable()){
					if (sector_schedule[s_nbr].update(timestamp) != sector_active_prev[s_nbr]){
						if (sector_schedule[s_nbr].isActive()){
							HAL_FatFs_Logger::publishLogMessage("Irrigation start", irg_logs_box, static_cast<reporter_t>(s_nbr), log_msg_prio_t::MEDIUM);
							HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
							p_sector[s_nbr]->setWateringState(true);
						}
						else{
							HAL_FatFs_Logger::publishLogMessage("Irrigation stop", irg_logs_box, static_cast<reporter_t>(s_nbr), log_msg_prio_t::MEDIUM);
							HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
							p_sector[s_nbr]->setWateringState(false);
						}
						sector_active_prev[s_nbr] = sector_schedule[s_nbr].isActive();
					}
				}
			}
		}
		else{
			HAL_FatFs_Logger::publishLogMessage("Irrigation stop, tank error", irg_logs_box, reporter_t::Watertank1, log_msg_prio_t::CRITICAL);
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
