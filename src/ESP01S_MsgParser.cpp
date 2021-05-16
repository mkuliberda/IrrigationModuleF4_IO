#include "Esp01s_MsgParser.h"

bool Esp01s_MsgParser::parseString(const std::string& _str, void(*callback)(const std::string&)) {

	if (_str.substr(2, 13) == "NTP999>SYS000") {
		if (_str.substr(19, 3) == "CTD") {
			callback(_str);
			return true;
		}
	}
	return false;
}

bool Esp01s_MsgParser::parseString(const std::string& _str) {
	return false;
}

IncomingMessage Esp01s_MsgParser::parseIncoming(uint8_t *buffer, const size_t& _len)
{
	if (IncomingMessage msg{}; _len >= 24)
	{
		msg.sender.object = static_cast<ExternalObject_t>(buffer[0]);
		msg.sender.id = (buffer[1] << 8) | buffer[2];
		msg.recipient.object = static_cast<InternalObject_t>(buffer[3]);
		msg.recipient.id = (buffer[4] << 8) | buffer[5];
		msg.type = static_cast<MsgValueType>(buffer[6]);
		msg.msg_value1 = (buffer[7] << 8) | buffer[8];
		msg.msg_value2 = (buffer[9] << 8) | buffer[10];
		msg.msg_value3 = (buffer[11] << 8) | buffer[12];
		msg.msg_value4 = (buffer[13] << 8) | buffer[14];
		msg.msg_value5 = (buffer[15] << 8) | buffer[16];
		msg.msg_value6 = (buffer[17] << 8) | buffer[18];
		msg.msg_value7 = (buffer[19] << 8) | buffer[20];
		msg.msg_value8 = (buffer[21] << 8) | buffer[22];
		msg.end = buffer[23];
		return msg;
	}
	return {};

}