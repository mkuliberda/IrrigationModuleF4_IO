#ifndef ESP01S_MsgParser_H_
#define ESP01S_MsgParser_H_

#include "MsgParser.h"

class ESP01S_MsgParser: public MsgParser{
public:
    ESP01S_MsgParser() =default;
    bool parseString(const std::string& _str) override;
    bool parseString(const std::string& _str, void(*action)(const std::string&)) override;

    ESP01S_MsgParser(ESP01S_MsgParser const &) = delete;
	ESP01S_MsgParser& operator=(ESP01S_MsgParser const&) = delete;
    ~ESP01S_MsgParser() =default;
};

#endif