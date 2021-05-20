#include "MsgBrokerFactory.h"
#include "HAL_UART_DMA_MsgBroker.h"
#include "LL_UART_DMA_SIM800L_MsgBroker.h"
#include <memory>

MsgBrokerPtr MsgBrokerFactory::create(const MsgBrokerType& _type, void *_dev_handle, void *_periph_handle)
{
	switch (_type) {
	case MsgBrokerType::hal_uart_dma:
		return std::make_unique<HAL_UART_DMA_MsgBroker>(_dev_handle);
	case MsgBrokerType::ll_uart_dma_sim800l:
		return std::make_unique<LL_UART_DMA_SIM800L_MsgBroker>(_dev_handle, _periph_handle);
	default:
		return nullptr;
	}
}