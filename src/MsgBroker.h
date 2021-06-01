#ifndef __MsgBroker_H
#define __MsgBroker_H

#pragma once
#include <map>
#include <string>
#include <memory>
#include <unordered_map>
#include "Messages.h"
#include "MsgEncoder.h"
#include "MsgParser.h"
#include "stm32f4xx_hal.h"


class MsgBroker
{
public:
	virtual bool sendMsg(const ExternalObject& _recipient, const InternalObject& _publisher, const std::string& _msg, const bool& _wait_until_cplt, Encoder *_encoder = nullptr) =0;
	virtual bool publishData(const ExternalObject& _recipient, const InternalObject& _publisher, std::unordered_map<std::string, int32_t> _values, const bool& _wait_until_cplt, Encoder *_encoder = nullptr) =0;
	virtual bool requestData(const ExternalObject& _recipient, const InternalObject& _publisher, const std::string& _data_key, const std::string& _data_type, const bool& _wait_until_cplt = false, Encoder *_encoder = nullptr) =0;
	virtual bool read() =0;
	virtual IncomingMessage getIncoming(MsgParser *_parser = nullptr) =0;
	virtual bool setMsgLength(const size_t& _msg_len = -1) =0;
	virtual	size_t getMsgLength() const =0;
	virtual bool setDefaultParser(MsgParser *_parser) =0;
	virtual bool setDefaultEncoder(Encoder *_encoder) =0;
	virtual void setExternalAddresses(std::unordered_map<ExternalObject_t, std::string> *_addresses) = 0;
	virtual void setInternalAddresses(std::unordered_map<InternalObject_t, std::string> *_addresses) = 0;
	virtual ~MsgBroker() =default;
};

using MsgBrokerPtr = std::unique_ptr<MsgBroker>; //alias for MsgBroker pointer

#endif