#pragma once
#include "MsgParser.h"
class Sms_MsgParser :
	public MsgParser
{
public:
	Sms_MsgParser() = default;
	IncomingMessage parseIncoming(uint8_t *buffer, const size_t& _len) override;
	bool parseString(const std::string& _str) override;
	bool parseString(const std::string& _str, void(*callback)(const std::string&)) override;

	Sms_MsgParser(Sms_MsgParser const &) = delete;
	Sms_MsgParser& operator=(Sms_MsgParser const&) = delete;
	~Sms_MsgParser() override = default;
};

