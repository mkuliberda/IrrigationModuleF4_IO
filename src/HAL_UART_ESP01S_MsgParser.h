#ifndef HAL_UART_ESP01S_MsgParser_H_
#define HAL_UART_ESP01S_MsgParser_H_

#include "Parser.h"

class HAL_UART_ESP01S_MsgParser: public Parser{
public:
    bool parseString(const std::string& _str) override;
    ~HAL_UART_ESP01S_MsgParser() =default;

private:
    void setRtc(const std::string& _str);
};

#endif