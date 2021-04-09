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
       if (_devHandle != nullptr) UART_Handle = (UART_HandleTypeDef*)_devHandle;
    }
	~HAL_UART_MsgBroker() =default;
	bool assignDevice(void* DevHandle) override;
	bool sendMsg(const recipient_t& _recipient, const std::string& _msg) override;
	bool publishData(const recipient_t& _recipient, const char* _publisher, std::unordered_map<std::string, int32_t> _values) override;
	bool requestData(const recipient_t& _recipient, const std::string& _data_key) override;
private:
    bool transmit(const std::string& _str); 
	const char *pub_hdr = "PUB\\";
	const char *get_hdr = "GET\\";
	UART_HandleTypeDef *UART_Handle{};
	uint8_t txBuffer[BUFFER_SIZE]{};
};

#endif

