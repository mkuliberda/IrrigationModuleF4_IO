#ifndef __SIM800L_MSGBROKER_H
#define __SIM800L_MSGBROKER_H

#pragma once

#include "MsgBroker.h"
#include <iostream>
#include <string>
#include "stm32f4xx_hal.h"
#include "LL_UART_DMA_SIM800L.h"


constexpr size_t uart_dma_sim800l_tx_buffer_size{ 64 };
constexpr size_t uart_dma_sim800l_rx_buffer_size{ 64 };

class SIM800L_MsgBroker :
	public MsgBroker
{
public:

	explicit SIM800L_MsgBroker(void* _dev_handle, void* _periph_handle)
	{
		if (_periph_handle != nullptr){
		   uart_handle = static_cast<UART_HandleTypeDef*>(_periph_handle);
		   periph_valid = true;
		}
		if (_dev_handle != nullptr){
		   dev_handle = static_cast<GSMModule*>(_periph_handle);
		   periph_valid = true;
		}
    }
	bool sendMsg(const ExternalObject& _recipient, const InternalObject& _publisher, const std::string& _msg, const bool& _wait_until_cplt, Encoder *_encoder = nullptr) override;
	bool publishData(const ExternalObject& _recipient, const InternalObject& _publisher, std::unordered_map<std::string, int32_t> _values, const bool& _wait_until_cplt, Encoder *_encoder = nullptr) override;
	bool requestData(const ExternalObject& _recipient, const InternalObject& _publisher, const std::string& _data_key, const std::string& _data_type, const bool& _wait_until_cplt, Encoder *_encoder = nullptr) override;
	bool setDefaultParser(MsgParser *_parser) override;
	bool setDefaultEncoder(Encoder *_encoder) override;
	void setExternalAddresses(std::unordered_map<ExternalObject_t, std::string> *_addresses) override;
	void setInternalAddresses(std::unordered_map<InternalObject_t, std::string> *_addresses) override;
	bool read() override;
	bool setMsgLength(const size_t& _msg_len = -1) override;
	size_t getMsgLength() const  override;
	IncomingMessage getIncoming(MsgParser *_parser = nullptr) override;
	SIM800L_MsgBroker(SIM800L_MsgBroker const &) = delete;
	SIM800L_MsgBroker& operator=(SIM800L_MsgBroker const&) = delete;
	~SIM800L_MsgBroker() =default;

private:
	UART_HandleTypeDef *uart_handle{};
	GSMModule *dev_handle{};
	//uint8_t tx_buffer[uart_dma_sim800l_tx_buffer_size]{};
	//std::string rx_buffer{};
	std::unordered_map<ExternalObject_t, std::string> *ext_address_map{};
	std::unordered_map<InternalObject_t, std::string> *int_address_map{};
	//IncomingMessage msg_in{};
	MsgParser *parser{};
	Encoder *encoder{};
	bool dev_valid{false};
	bool periph_valid{false};
	size_t msg_len{};
	size_t old_pos;
};

#endif