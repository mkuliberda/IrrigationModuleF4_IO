#include "GSMModule.h"
#include "stm32f4xx_hal.h"
#include "gpio.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_dma.h"

enum class SIM800L_RegistrationStatus{UNKNOWN, HOME, ROAMING};

enum class SIM800L_NetworkRegistration {NOT_REGISTERED, REGISTERED_HOME, SEARCHING, DENIED, NET_UNKNOWN, REGISTERED_ROAMING, NET_ERROR};

enum class SIM800L_CharacterSet{GSM_format, UCS2, HEX, IRA, PCCP, PCDN, _8859_1};

enum class SIM800L_Attention{
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
    SET_SMS_MODE_TEXT,
    SET_SMS_MODE_PDU,
    NEW_SMS_BEGIN,
    GET_LOCAL_TIME,
    SET_CMD_ECHO_OFF,
    SET_CMD_ECHO_ON,
    GET_SUPPORTED_CHAR_SET,
    GET_CURRENT_CHAR_SET,
    GET_LAST_EXT_ERROR,
    GET_EXT_ERROR_REPORTS_SETTING,
    SET_CHAR_SET,
    SET_EXT_ERROR_REPORTS_STR,
    SET_EXT_ERROR_REPORTS_INT,
    GET_NETWORK_REG_STATUS,
    SET_NETWORK_REG_DISABLE,
    SET_NETWORK_REG_ENABLE_WO_LOC,
    SET_NETWORK_REG_ENABLE_WITH_LOC,
    READ_SMS_AT_INDEX
};

inline std::unordered_map<SIM800L_Attention, std::string> at_query{  // NOLINT(clang-diagnostic-exit-time-destructors)
	{SIM800L_Attention::GET_SUPPORTED_CHAR_SET, "AT+CSCS=?\r"},
    {SIM800L_Attention::GET_CURRENT_CHAR_SET, "AT+CSCS?\r"},
    {SIM800L_Attention::GET_LOCAL_TIME, "AT+CCLK?\r"},
    {SIM800L_Attention::GET_SMS_TEXT_MODE, "AT+CMGF?\r"},
    {SIM800L_Attention::GET_LAST_EXT_ERROR, "AT+CEER\r"},
    {SIM800L_Attention::GET_EXT_ERROR_REPORTS_SETTING, "AT+CEER?\r"},
    {SIM800L_Attention::GET_NETWORK_REG_STATUS, "AT+CGREG?\r"}
};

inline std::unordered_map<SIM800L_Attention, std::string> at_command{  // NOLINT(clang-diagnostic-exit-time-destructors)
    {SIM800L_Attention::SET_SMS_MODE_TEXT, "AT+CMGF=1\r"},
    {SIM800L_Attention::SET_SMS_MODE_PDU, "AT+CMGF=0\r"},
    {SIM800L_Attention::NEW_SMS_BEGIN, "AT+CMGS=\""},
    {SIM800L_Attention::READ_SMS_AT_INDEX, "AT+CMGR="},
    {SIM800L_Attention::SET_CMD_ECHO_ON, "ATE1\r"},
    {SIM800L_Attention::SET_CMD_ECHO_OFF, "ATE0\r"},
    {SIM800L_Attention::SET_CHAR_SET, "AT+CSCS="},
    {SIM800L_Attention::SET_EXT_ERROR_REPORTS_STR, "AT+CEER=0\r"},
    {SIM800L_Attention::SET_EXT_ERROR_REPORTS_INT, "AT+CEER=1\r"},
    {SIM800L_Attention::SET_NETWORK_REG_DISABLE, "AT+CGREG=0\r"},
    {SIM800L_Attention::SET_NETWORK_REG_ENABLE_WO_LOC, "AT+CGREG=1\r"},
    {SIM800L_Attention::SET_NETWORK_REG_ENABLE_WITH_LOC, "AT+CGREG=2\r"}
};

class SIM800LConfiguration : public GSMModuleConfiguration{
public:
    explicit SIM800LConfiguration(const bool& _echo_on=true, const SIM800L_CharacterSet& _char_set=SIM800L_CharacterSet::GSM_format, const bool& _ext_err_report_type_str=true)
    {
        configuration_vect.emplace_back( _ext_err_report_type_str == true ? at_command[SIM800L_Attention::SET_EXT_ERROR_REPORTS_STR] : at_command[SIM800L_Attention::SET_EXT_ERROR_REPORTS_INT]); 
 
        switch (_char_set){
            case SIM800L_CharacterSet::GSM_format:
                configuration_vect.emplace_back(at_command[SIM800L_Attention::SET_CHAR_SET] + "GSM\r");
                break;
            case SIM800L_CharacterSet::HEX:
                configuration_vect.emplace_back(at_command[SIM800L_Attention::SET_CHAR_SET] + "HEX\r");
                break;
            case SIM800L_CharacterSet::IRA:
                configuration_vect.emplace_back(at_command[SIM800L_Attention::SET_CHAR_SET] + "IRA\r");
                break;
            case SIM800L_CharacterSet::PCCP:
                configuration_vect.emplace_back(at_command[SIM800L_Attention::SET_CHAR_SET] + "PCCP\r");
                break;
            case SIM800L_CharacterSet::PCDN:
                configuration_vect.emplace_back(at_command[SIM800L_Attention::SET_CHAR_SET] + "PCDN\r");
                break;
            case SIM800L_CharacterSet::UCS2:
                configuration_vect.emplace_back(at_command[SIM800L_Attention::SET_CHAR_SET] + "UCS2\r");
                break;
            case SIM800L_CharacterSet::_8859_1:
                configuration_vect.emplace_back(at_command[SIM800L_Attention::SET_CHAR_SET] + "8859-1\r");
                break;
            default:
                break;
        };
        configuration_vect.emplace_back( _echo_on == true ? at_command[SIM800L_Attention::SET_CMD_ECHO_ON] : at_command[SIM800L_Attention::SET_CMD_ECHO_OFF]); 
    };

    std::vector<std::string> getConfiguration() override { return configuration_vect; };

private:
    std::vector<std::string> configuration_vect{};
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
    ProcessResult configure(void *_config) override;
    bool isReady() override;
    ModuleState getState() override;
    uint32_t getSignal() override;
    RegistrationStatus getNetworkRegistrationStatus() override;
    ProcessResult reset() override;
    ProcessResult sendMessage(const std::string_view& _mobile_number, const std::string_view& text) override;
    std::string readMessage(const uint32_t& _index) override;
    std::string getMessageOrigin(const uint32_t& _index) override;
    ProcessResult deleteAllMessages() override;
	 ~LL_UART_DMA_SIM800L() =default;

protected:
    void processRxEvent();
	ProcessResult transmit(const std::string& _str); 
    ProcessResult transmit(const char& ch);
	void accumulateRxBuffer(const void* data, size_t len);
    ProcessResult read(const std::string& _expected_ok="OK", const std::string& _expected_error="ERROR");
    std::string readAnswer(const std::string& _expected_ok="OK", const std::string& _expected_error="ERROR");
    std::vector<std::string> tokenizeString(const std::string &s, const std::string &delim);

private:
    USART_TypeDef* uart_handle{};
    GSMModuleConfiguration* config_handle{};
    struct gpio_s rst_pin{};
    struct gpio_s ri_pin{};
    bool periph_valid{false};
    size_t old_pos{};
    std::string rx_buffer{};
    const uint32_t max_cmd_len{556+2};
    ModuleState state;
};