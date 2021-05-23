#include "sim800l.h"
#include "stm32f4xx_hal.h"


class LL_UART_DMA_SIM800L : public SIM800L{
public:
    bool assignPeripheral(void *_periph_handle) override;
    bool configure() override;
    std::string read() override;
    bool isReady() override;
    uint32_t getSignal() override;
    SIM800L_RegistrationStatus getRegistrationStatus() override;
    void reset() override;
    bool sendSms(const std::string_view& _mobile_number, const std::string_view& text) override;
    std::string readSms(const uint32_t& _index) override;
    std::string getSmsSender(const uint32_t& _index) override;
    bool deleteAllSms() override;
	 ~LL_UART_DMA_SIM800L() =default;

private:
    UART_HandleTypeDef* uart_handle{};
    bool periph_valid{false};
};