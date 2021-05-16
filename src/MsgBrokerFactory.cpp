#include "MsgBrokerFactory.h"
#include "HAL_UART_DMA_MsgBroker.h"
#include <memory>

MsgBrokerPtr MsgBrokerFactory::create(const MsgBrokerType& _type, const size_t& _msg_in_len, void *_dev_handle)
{
	switch (_type) {
	case MsgBrokerType::hal_uart_dma:
		return std::make_unique<HAL_UART_DMA_MsgBroker>(_msg_in_len, _dev_handle);
	default:
		return nullptr;
	}
}