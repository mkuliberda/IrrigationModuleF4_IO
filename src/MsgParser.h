#ifndef MSG_PARSER_H_
#define MSG_PARSER_H_

#include <string>
#include "Messages.h"


class MsgParser {
public:
	virtual IncomingMessage parseIncoming(uint8_t *buffer, const size_t& _len) =0;
	virtual bool parseString(const std::string& _str) =0;
	virtual bool parseString(const std::string& _str, void(*callback)(const std::string&)) =0;
	virtual ~MsgParser() = default;
};

#endif

