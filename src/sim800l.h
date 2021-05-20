#include <string>

class LL_UART_DMA_SIM800L{
public:
    bool configure();
    std::string read();
	 ~LL_UART_DMA_SIM800L() =default;
};