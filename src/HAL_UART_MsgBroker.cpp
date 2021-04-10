#include "HAL_UART_MsgBroker.h"
#include "utilities.h"
#include <string>
#include <cstring>
#include "cmsis_os.h"

extern const UBaseType_t xArrayIndex;

bool HAL_UART_MsgBroker::assignDevice(void * DevHandle)
{
	if (DevHandle == nullptr) return false;
	UART_Handle = (UART_HandleTypeDef*)DevHandle;
	return true;
}

bool HAL_UART_MsgBroker::sendMsg(const recipient_t& _recipient, const std::string& _msg)
{
	std::string str_msg{pub_hdr};
	const std::string msg_start{"{\"Msg\":\""};
	const std::string msg_end{"\"}\n"};
	bool result = false;

	switch (_recipient) {
	case recipient_t::raspberry_pi:
		str_msg += RASPBERRY_PI_STR + msg_start + _msg + msg_end;
        result = transmit(str_msg);
		break;
	case recipient_t::google_home:
		str_msg += GOOGLE_HOME_STR + msg_start + _msg + msg_end;
        result = transmit(str_msg);
		break;
	case recipient_t::broadcast:
		str_msg += BROADCAST_STR + msg_start + _msg + msg_end;
        result = transmit(str_msg);
		break;
	default:
		break;
	}
	return result;
}

bool HAL_UART_MsgBroker::publishData(const recipient_t& _recipient, const char* _publisher, std::unordered_map<const char*, int32_t> _values)
{
	std::string str_msg{ "{\"" };
	str_msg += _publisher;
	str_msg += "\":[";
	bool result = false;

	for (const auto& kv : _values) {
		str_msg += '\"';
		str_msg += kv.first;
		str_msg += '\"';
		str_msg += ':';
		str_msg += patch::to_string(kv.second);
		str_msg += ",";
	}
	str_msg.pop_back();
	str_msg += "]}\n";

	switch (_recipient) {
	case recipient_t::raspberry_pi:
		str_msg = (static_cast<std::string>(pub_hdr) + RASPBERRY_PI_STR + str_msg);
        result = transmit(str_msg);
		break;
	case recipient_t::google_home:
		str_msg = (static_cast<std::string>(pub_hdr) + GOOGLE_HOME_STR + str_msg);
		result = transmit(str_msg);
		break;
	case recipient_t::broadcast:
		str_msg = (static_cast<std::string>(pub_hdr) + BROADCAST_STR + str_msg);
		result = transmit(str_msg);
		break;
	default:
		break;
	}
	return result;
}

bool HAL_UART_MsgBroker::requestData(const recipient_t& _recipient, const std::string& _data_key)
{
	std::string str_msg{ get_hdr };
	bool result = false;

	switch (_recipient) {
	case recipient_t::raspberry_pi:
		str_msg += RASPBERRY_PI_STR + _data_key;
		result = transmit(str_msg);
		break;
	case recipient_t::google_home:
		str_msg += GOOGLE_HOME_STR + _data_key;
		result = transmit(str_msg);
		break;
	case recipient_t::ntp_server:
		str_msg += NTP_SERVER_STR + _data_key;
        result = transmit(str_msg);
		break;
	default:
		break;
	}
	return result;
}

bool HAL_UART_MsgBroker::transmit(const std::string& _str){
	int32_t bytes_to_send = _str.length();
	uint32_t pos = 0;
	uint8_t retries = 0;
	bool success = false;
	while(bytes_to_send > 0){
		std::memset(txBuffer, '\0', BUFFER_SIZE);
		_str.copy((char*)txBuffer, MINIMUM(bytes_to_send, BUFFER_SIZE), pos);
		if (HAL_UART_Transmit_DMA(UART_Handle, txBuffer, MINIMUM(bytes_to_send, BUFFER_SIZE)) == HAL_OK){
			ulTaskNotifyTake( xArrayIndex, osWaitForever);
			bytes_to_send -= BUFFER_SIZE;
			pos += BUFFER_SIZE;
			success = true;
			retries = 0;
		}
		else{
			retries++;
			success = false;
		}
		if (retries > 20) break;
	}
	return success;
} 


