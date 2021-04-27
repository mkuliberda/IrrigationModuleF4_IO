#ifndef SMS_MsgParser_H_
#define SMS_MsgParser_H_

#include "MsgParser.h"

class SMS_MsgParser: public MsgParser{
public:
    SMS_MsgParser() =default;
    bool parseString(const std::string& _str) override;
    bool parseString(const std::string& _str, void(*action)(const std::string&)) override;
    SMS_MsgParser(SMS_MsgParser const &) =delete;
	SMS_MsgParser& operator=(SMS_MsgParser const&) =delete;
    ~SMS_MsgParser() =default;
};

#endif