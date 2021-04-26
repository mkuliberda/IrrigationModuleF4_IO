#ifndef MSG_PARSER_H_
#define MSG_PARSER_H_

#include <string>

class MsgParser{
public:
    virtual bool parseString(const std::string& _str) =0;
    virtual bool parseString(const std::string& _str, void(*action)(const std::string&)) =0;
    virtual ~MsgParser() =default;
};

#endif