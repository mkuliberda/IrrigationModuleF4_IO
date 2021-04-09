#include "MsgBrokerFactory.h"
#include "HAL_UART_MsgBroker.h"
#include <memory>

MsgBrokerPtr MsgBrokerFactory::create(const msg_broker_type_t & _type, void* _devHandle)
{
	switch (_type) {
	case msg_broker_type_t::hal_uart:
		return std::make_unique<HAL_UART_MsgBroker>(_devHandle);
	default:
		return nullptr;
	}
}