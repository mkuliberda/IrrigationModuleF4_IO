#ifndef __HAL_UART_MsgBroker_H
#define __HAL_UART_MsgBroker_H

#pragma once

#include "MsgBroker.h"
#include <iostream>
#include <string>
#include "stm32f4xx_hal.h"

constexpr size_t uart_dma_tx_buffer_size{ 64 };
constexpr size_t uart_dma_rx_buffer_size{ 64 };

class HAL_UART_DMA_MsgBroker :
	public MsgBroker
{
public:

	HAL_UART_DMA_MsgBroker(void* _dev_handle)
	{
		if (_dev_handle != nullptr){
		   uart_handle = static_cast<UART_HandleTypeDef*>(_dev_handle);
		   dev_valid = true;
		}
    }
	bool assignDevice(void* _dev_handle) override;
	bool sendMsg(const ExternalObject& _recipient, const InternalObject& _publisher, const std::string& _msg, const bool& _wait_until_cplt, Encoder *_encoder = nullptr) override;
	bool publishData(const ExternalObject& _recipient, const InternalObject& _publisher, std::unordered_map<std::string, int32_t> _values, const bool& _wait_until_cplt, Encoder *_encoder = nullptr) override;
	bool requestData(const ExternalObject& _recipient, const InternalObject& _publisher, const std::string& _data_key, const std::string& _data_type, const bool& _wait_until_cplt, Encoder *_encoder = nullptr) override;
	bool setDefaultParser(MsgParser *_parser) override;
	bool setDefaultEncoder(Encoder *_encoder) override;
	void setExternalAddresses(std::unordered_map<ExternalObject_t, std::string> *_addresses) override;
	void setInternalAddresses(std::unordered_map<InternalObject_t, std::string> *_addresses) override;
	bool read() override;
	bool setMsgLength(const size_t& _msg_len = -1) override;
	size_t& getMsgLength() override;
	IncomingMessage getIncoming(MsgParser *_parser = nullptr) override;
	HAL_UART_DMA_MsgBroker(HAL_UART_DMA_MsgBroker const &) = delete;
	HAL_UART_DMA_MsgBroker& operator=(HAL_UART_DMA_MsgBroker const&) = delete;
	~HAL_UART_DMA_MsgBroker() =default;


private:
	bool transmit(const std::string& _str, const bool& _blocking_mode); 
	UART_HandleTypeDef *uart_handle{};
	uint8_t tx_buffer[uart_dma_tx_buffer_size]{};
	uint8_t rx_buffer[uart_dma_rx_buffer_size]{};
	std::unordered_map<ExternalObject_t, std::string> *ext_address_map{};
	std::unordered_map<InternalObject_t, std::string> *int_address_map{};
	IncomingMessage msg_in{};
	MsgParser *parser{};
	Encoder *encoder{};
	bool dev_valid{false};
	size_t msg_len;
};

#endif