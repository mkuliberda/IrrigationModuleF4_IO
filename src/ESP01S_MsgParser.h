#pragma once
#include "MsgParser.h"

class Esp01s_MsgParser :
	public MsgParser
{
public:
	Esp01s_MsgParser() = default;
	IncomingMessage parseIncoming(uint8_t *buffer, const size_t& _len) override;
	bool parseString(const std::string& _str) override;
	bool parseString(const std::string& _str, void(*callback)(const std::string&)) override;

	Esp01s_MsgParser(Esp01s_MsgParser const &) = delete;
	Esp01s_MsgParser& operator=(Esp01s_MsgParser const&) = delete;
	~Esp01s_MsgParser() override =default;
};

