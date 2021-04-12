#ifndef __MsgBroker_H
#define __MsgBroker_H

#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include "../lib/Parser/parser.h"

#define RASPBERRY_PI_STR "rpi\\"
#define GOOGLE_HOME_STR "goo\\"
#define NTP_SERVER_STR "ntp\\"
#define BROADCAST_STR "all\\"


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
	virtual bool sendMsg(const recipient_t& _recipient, const std::string& _msg, const bool& _wait_until_cplt = false) =0;
	virtual bool publishData(const recipient_t& _recipient, const char* _publisher, std::unordered_map<const char*, int32_t> _values, const bool& _wait_until_cplt = false) =0;
	virtual bool requestData(const recipient_t& _recipient, const std::string& _data_key, const bool& _wait_until_cplt = false) =0;
	virtual bool readData(const size_t& _size) =0;
	virtual bool setParser(Parser *_parser) =0;
	virtual ~MsgBroker() =default;

};

using MsgBrokerPtr = std::unique_ptr<MsgBroker>; //alias for MsgBroker pointer

#endif