#include "ESP01S_MsgParser.h"
#include <string>

bool ESP01S_MsgParser::parseString(const std::string& _str, void(*action)(const std::string&)){
    
    if(_str.substr(2,13) == "NTP999>SYS000"){
        if (_str.substr(19,3) == "CTD"){
            action(_str);
            return true;
        }
    }
    return false;
}

bool ESP01S_MsgParser::parseString(const std::string& _str){
    return false;
}
