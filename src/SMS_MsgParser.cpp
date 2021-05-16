#include "Sms_MsgParser.h"

IncomingMessage Sms_MsgParser::parseIncoming(uint8_t* buffer, const size_t& _len)
{
	return {};
}

bool Sms_MsgParser::parseString(const std::string& _str)
{
	return false;
}

bool Sms_MsgParser::parseString(const std::string& _str, void(*callback)(const std::string&))
{
	return false;
}