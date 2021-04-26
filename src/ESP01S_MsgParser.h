#ifndef ESP01S_MsgParser_H_
#define ESP01S_MsgParser_H_

#include "MsgParser.h"

class ESP01S_MsgParser: public MsgParser{
public:
    bool parseString(const std::string& _str) override;
    bool parseString(const std::string& _str, void(*action)(const std::string&)) override;
    ~ESP01S_MsgParser() =default;
};

#endif