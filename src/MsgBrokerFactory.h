#ifndef __MsgBrokerFactory_H
#define __MsgBrokerFactory_H

#pragma once
#include "MsgBroker.h"
#include <string_view>

enum msg_broker_type_t {
	hal_uart
};

class MsgBrokerFactory
{
public:
	static MsgBrokerPtr create(const msg_broker_type_t& _type, void* _devHandle=nullptr);
};

#endif