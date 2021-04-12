#ifndef __HAL_UART_MsgBroker_H
#define __HAL_UART_MsgBroker_H

#pragma once

#include "MsgBroker.h"
#include <iostream>
#include <string>
#include "stm32f4xx_hal.h"

#define BUFFER_SIZE 64

class HAL_UART_MsgBroker :
	public MsgBroker
{
public:
    HAL_UART_MsgBroker(void* _devHandle){
       if (_devHandle != nullptr){
		   UART_Handle = (UART_HandleTypeDef*)_devHandle;
		   devValid = true;
	   } 
    }
	~HAL_UART_MsgBroker() =default;
	bool assignDevice(void* DevHandle) override;
	bool sendMsg(const recipient_t& _recipient, const std::string& _msg, const bool& _wait_until_cplt = false) override;
	bool publishData(const recipient_t& _recipient, const char* _publisher, std::unordered_map<const char*, int32_t> _values, const bool& _wait_until_cplt = false) override;
	bool requestData(const recipient_t& _recipient, const std::string& _data_key, const bool& _wait_until_cplt = false) override;
	bool setParser(Parser *_parser);
	bool readData(const size_t& _size);

private:
    bool transmit(const std::string& _str, const bool& _blocking_mode); 
	const char *pub_hdr = "$PUB\\";
	const char *get_hdr = "$GET\\";
	UART_HandleTypeDef *UART_Handle{};
	uint8_t txBuffer[BUFFER_SIZE]{};
	uint8_t rxBuffer[BUFFER_SIZE]{};
	Parser *parserInstance{};
	bool devValid{false};
};

#endif

