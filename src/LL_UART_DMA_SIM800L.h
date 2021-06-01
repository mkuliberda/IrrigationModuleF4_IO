#include "GSMModule.h"
#include "stm32f4xx_hal.h"
#include "gpio.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_dma.h"

enum class SIM800L_RegistrationStatus{UNKNOWN, HOME, ROAMING};

enum class SIM800L_NetworkRegistration {NOT_REGISTERED, REGISTERED_HOME, SEARCHING, DENIED, NET_UNKNOWN, REGISTERED_ROAMING, NET_ERROR};

enum class SIM800L_CharacterSet{GSM_format, UCS2, HEX, IRA, PCCP, PCDN, _8859_1};

enum class SIM800L_Command{
    GET_CHAR_SET,
    REPEAT_LAST_CMD,
    ASNWER_INCOMING_CALL,
    REDIAL,
    DISCONNECT_CONNECTION,
    GET_PROD_INFO,
    SWITCH_FROM_DATA_MODE_TO_CMD_MODE,
    SWTICH_FROM_CMD_MODE_TO_DATA_MODE,
    SET_CMD_LINE_TERMINATION_CHAR,
    SET_RESPONSE_FORMATTING_CHAR,
    SET_CMD_LINE_EDITING_CHAR,
    TA_RESPONSE_FORMAT,
    RESET_DEFAULT,
    SET_DCD_MODE,
    SET_DTR_MODE,
    DISP_CURRENT_CONFIG,
    GET_IMEI,
    GET_SMS_TEXT_MODE,
    SET_SMS_TEXT_MODE,
    SET_SMS_PDU_MODE,
    NEW_SMS,
    GET_LOCAL_TIME,
    SET_CMD_ECHO_OFF,
    SET_CMD_ECHO_ON
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
	{SIM800L_Command::GET_CHAR_SET, "AT+CSCS?"},
    {SIM800L_Command::SET_SMS_TEXT_MODE, "AT+CMGF=1"},
    {SIM800L_Command::SET_SMS_PDU_MODE, "AT+CMGF=0"},
    {SIM800L_Command::NEW_SMS, "AT+CMGS=\""},
    {SIM800L_Command::GET_LOCAL_TIME, "AT+CCLK=?"},
    {SIM800L_Command::GET_SMS_TEXT_MODE, "AT+CMGF?"},
    {SIM800L_Command::SET_CMD_ECHO_ON, "ATE1"},
    {SIM800L_Command::SET_CMD_ECHO_OFF, "ATE0"},
};

class SIM800LConfiguration : public GSMModuleConfiguration{
public:
    explicit SIM800LConfiguration(const bool& _echo_on, const SIM800L_CharacterSet& _char_set ):
    cmd_echo_on(_echo_on),
    char_set(_char_set)
    {
        switch (this->char_set){
            case SIM800L_CharacterSet::GSM_format:
            configuration.emplace_back(" ");
            break;
        case SIM800L_CharacterSet::HEX:
        break;
        case SIM800L_CharacterSet::IRA:
        break;
        case SIM800L_CharacterSet::PCCP:
        break;
        case SIM800L_CharacterSet::PCDN:
        break;
        case SIM800L_CharacterSet::UCS2:
        break;
        case SIM800L_CharacterSet::_8859_1:
        break;
        };
    };
    std::vector<std::string> getConfiguration () override;

private:
    bool cmd_echo_on{true};
    SIM800L_CharacterSet char_set{SIM800L_CharacterSet::GSM_format};
    std::vector<std::string> configuration;
};

class LL_UART_DMA_SIM800L : public GSMModule{
public:
    explicit LL_UART_DMA_SIM800L(void * _periph, const struct gpio_s& _rst_pin, const struct gpio_s& _ri_pin = {0, 0}){
       if (_periph != nullptr) {
           uart_handle = static_cast<USART_TypeDef*>(_periph);
           periph_valid = true;
       }
       rst_pin.pin = _rst_pin.pin;
       rst_pin.port = _rst_pin.port;
       ri_pin.pin = _ri_pin.pin;
       ri_pin.port = _ri_pin.port;
    }
    bool configure(void *_config) override;
    bool isReady() override;
    uint32_t getSignal() override;
    RegistrationStatus getRegistrationStatus() override;
    void reset() override;
    bool sendSms(const std::string_view& _mobile_number, const std::string_view& text) override;
    std::string readSms(const uint32_t& _index) override;
    std::string getSmsSender(const uint32_t& _index) override;
    bool deleteAllSms() override;
	 ~LL_UART_DMA_SIM800L() =default;

private:
    bool read();
	bool transmit(const std::string& _str); 
	std::string processBuffer(const void* data, size_t len);
    USART_TypeDef* uart_handle{};
    GSMModuleConfiguration *config_handle{};
    struct gpio_s rst_pin{};
    struct gpio_s ri_pin{};
    bool periph_valid{false};
    size_t old_pos{};

    const uint32_t max_cmd_len{556+2};
    const char sms_termination{26};

};