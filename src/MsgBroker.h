#ifndef __MsgBroker_H
#define __MsgBroker_H

#pragma once
#include <string>
#include <memory>
#include <unordered_map>


enum recipient_t : uint8_t {
	ntp_server,
	raspberry_pi,
	google_home,
	my_phone,
	broadcast
};

class MsgBroker
{
public:
	virtual bool assignDevice(void* DevHandle) =0;
	virtual bool sendMsg(const recipient_t& _recipient, const std::string& _msg) =0;
	virtual bool publishData(const recipient_t& _recipient, const char* _publisher, std::unordered_map<const char*, int32_t> _values) =0;
	virtual bool requestData(const recipient_t& _recipient, const std::string& _data_key) =0;
	virtual ~MsgBroker() =default;

};

using MsgBrokerPtr = std::unique_ptr<MsgBroker>; //alias for MsgBroker pointer

#endif