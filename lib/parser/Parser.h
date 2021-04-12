#ifndef PARSER_H_
#define PARSER_H_

#include <string>

class Parser{
public:
    virtual bool parseString(const std::string& _str) =0;
    virtual ~Parser() =default;
};

#endif