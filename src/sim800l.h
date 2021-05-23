#include <string>
#include <string_view>
#include <unordered_map>

enum class SIM800L_RegistrationStatus{UNKNOWN, HOME, ROAMING};

enum class SIM800L_NetworkRegistration {NOT_REGISTERED, REGISTERED_HOME, SEARCHING, DENIED, NET_UNKNOWN, REGISTERED_ROAMING, NET_ERROR};

enum class SIM800L_CharacterSet{GSM_format, UCS2, HEX, IRA, PCCP, PCDN, _8859_1};

struct SIM800L_CommandSyntax{
    const char prefix[2] = {'A', 'T'};
    const char termination = '\r';
};

enum class SIM800L_Command{
    GET_CHAR_SET,
    REPEAT_LAST_CMD,
    ASNWER_INCOMING_CALL,
    REDIAL,
    SET_ECHO_MODE,
    DISCONNECT_CONNECTION,
    GET_PROD_INFO,
    SWITCH_FROM_DATA_MODE_TO_CMD_MODE,
    SWTICH_FROM_CMD_MODE_TO_DATA_MODE,
    SET_CMD_LINE_TERMINATION_CHAR,
    SET_RESPONSE_FORMATTING_CHAR,
    SET_CMD_LINE_EDITING_CHAR,
    TA_SRESPONSE_FORMAT,
    RESET_DEFAULT,
    SET_DCD_MODE,
    SET_DTR_MODE,
    DISP_CURRENT_CONFIG,
    GET_IMEI,
    SET_SMS_TEXT_MODE,
    SET_SMS_PDU_MODE,
    NEW_SMS

};

enum class SIM800L_Response{
    test1,
    test2
};

enum class SIM800L_ErrorCodes{
    ERROR,
    OK
};

inline std::unordered_map<SIM800L_Command, std::string> at_command{  // NOLINT(clang-diagnostic-exit-time-destructors)
	{SIM800L_Command::GET_CHAR_SET, "AT+CSCS"},
    {SIM800L_Command::SET_SMS_TEXT_MODE, "AT+CMGF=1\r"},
    {SIM800L_Command::SET_SMS_PDU_MODE, "AT+CMGF=0\r"},
    {SIM800L_Command::NEW_SMS, "AT+CMGS=\""}
    
};

inline std::unordered_map<SIM800L_Response, std::string> at_response{  // NOLINT(clang-diagnostic-exit-time-destructors)
	{SIM800L_Response::test1, "test"},
    {SIM800L_Response::test2, "test"}
};


class SIM800L{
public:
    virtual bool assignPeripheral(void *_periph_handle) =0;
    virtual bool configure() =0;
    virtual std::string read() =0;
    virtual bool isReady() =0;
    virtual uint32_t getSignal() =0;
    virtual SIM800L_RegistrationStatus getRegistrationStatus() =0;
    virtual void reset() =0;
    virtual bool sendSms(const std::string_view& _mobile_number, const std::string_view& text) =0;
    virtual std::string readSms(const uint32_t& _index) =0;
    virtual std::string getSmsSender(const uint32_t& _index) =0;
    virtual bool deleteAllSms() =0;
	virtual ~SIM800L() =default;

    const uint32_t max_cmd_len{556+2};
    const char sms_termination{26};
};
