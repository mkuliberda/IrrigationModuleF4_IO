#include "HAL_UART_DMA_MsgBroker.h"
#include "utilities.h"
#include <string>
#include <cstring>
#include "cmsis_os.h"

extern const UBaseType_t xArrayIndex;

bool HAL_UART_DMA_MsgBroker::assignDevice(void *_dev_handle)
{
	if (_dev_handle == nullptr) return false;
	uart_handle = static_cast<UART_HandleTypeDef*>(_dev_handle);
	return dev_valid = true;
}

bool HAL_UART_DMA_MsgBroker::sendMsg(const ExternalObject& _recipient, const InternalObject& _publisher, const std::string& _msg, const bool& _wait_until_cplt, Encoder *_encoder)
{
	if (_publisher.id > 999 || _recipient.id > 999) return false;

	std::string id_str = patch::to_string(_publisher.id);
	const std::string pub_id = std::string(3 - id_str.length(), '0') + id_str;
	id_str = patch::to_string(_recipient.id);
	const std::string rcv_id = std::string(3 - id_str.length(), '0') + id_str;

	const Header hdr{ int_address_map->at(_publisher.object) + pub_id,  ext_address_map->at(_recipient.object) + rcv_id };
	const DataItem msg{ "Info", _msg };
	const Footer end{ "", "" };

	if (_encoder != nullptr) {
		hdr.accept(*_encoder);
		msg.accept(*_encoder);
		end.accept(*_encoder);
		return transmit(_encoder->str(), _wait_until_cplt);
	}
	if (encoder != nullptr)
	{
		hdr.accept(*encoder);
		msg.accept(*encoder);
		end.accept(*encoder);
		return transmit(encoder->str(), _wait_until_cplt);
	}

	return false;
}

bool HAL_UART_DMA_MsgBroker::publishData(const ExternalObject& _recipient, const InternalObject& _publisher, std::unordered_map<std::string, int32_t> _values, const bool& _wait_until_cplt, Encoder *_encoder)
{
	if(_publisher.id > 999 || _recipient.id > 999) return false;

	std::string id_str = patch::to_string(_publisher.id);
	const std::string pub_id = std::string(3 - id_str.length(), '0') + id_str;
	id_str = patch::to_string(_recipient.id);
	const std::string rcv_id = std::string(3 - id_str.length(), '0') + id_str;
	
	Header hdr{ int_address_map->at(_publisher.object) + pub_id,  ext_address_map->at(_recipient.object) + rcv_id};
	Data data_list{};
	Footer end{ "", "" };
	
	for (auto &[key,value]: _values)
	{
		DataItem item{key, patch::to_string(value)};
		data_list.emplace_back(item);
	}

	if (std::vector<Element*> msg_items{ &hdr, &data_list, &end }; _encoder != nullptr) {
		for (auto &item : msg_items) item->accept(*_encoder);
		return transmit(_encoder->str(), _wait_until_cplt);
	}
	else if(encoder != nullptr)
	{
		for (auto &item : msg_items) item->accept(*encoder);
		return transmit(encoder->str(), _wait_until_cplt);
	}

	return false;
}

bool HAL_UART_DMA_MsgBroker::requestData(const ExternalObject& _recipient, const InternalObject& _publisher, const std::string& _data_key, const std::string& _data_type, const bool& _wait_until_cplt, Encoder *_encoder)
{
	if(_publisher.id > 999 || _recipient.id > 999) return false;

	std::string id_str = patch::to_string(_publisher.id);
	const std::string pub_id = std::string(3 - id_str.length(), '0') + id_str;
	id_str = patch::to_string(_recipient.id);
	const std::string rcv_id = std::string(3 - id_str.length(), '0') + id_str;

	const Header hdr{ int_address_map->at(_publisher.object) + pub_id,  ext_address_map->at(_recipient.object) + rcv_id };
	const DataItem msg{ _data_key, _data_type };
	const Footer end{ "", "" };

	if (_encoder != nullptr) {
		hdr.accept(*_encoder);
		msg.accept(*_encoder);
		end.accept(*_encoder);
		return transmit(_encoder->str(), _wait_until_cplt);
	}
	if (encoder != nullptr)
	{
		hdr.accept(*encoder);
		msg.accept(*encoder);
		end.accept(*encoder);
		return transmit(encoder->str(), _wait_until_cplt);
	}

	return false;
}

 bool HAL_UART_DMA_MsgBroker::setDefaultParser(MsgParser *_parser){
	if (_parser == nullptr) return false;
	delete this->parser;
 	parser= _parser;
 	return true;
 	
 }
 bool HAL_UART_DMA_MsgBroker::setDefaultEncoder(Encoder*_encoder){
	 if (_encoder == nullptr) return false;
	 delete this->encoder;
	 encoder = _encoder;
	 return true;
 }

void HAL_UART_DMA_MsgBroker::setExternalAddresses(std::unordered_map<ExternalObject_t, std::string> *_addresses)
{
	ext_address_map = _addresses;
}
void HAL_UART_DMA_MsgBroker::setInternalAddresses(std::unordered_map<InternalObject_t, std::string> *_addresses)
{
	int_address_map = _addresses;
}

bool HAL_UART_DMA_MsgBroker::read(){

	if (!dev_valid){
		return false;
	}
	memset(rx_buffer, '\0', uart_dma_rx_buffer_size);
	HAL_UART_Receive_DMA(uart_handle, rx_buffer, msg_len);
	ulTaskNotifyTake( xArrayIndex, osWaitForever);
	return true;
}

bool HAL_UART_DMA_MsgBroker::transmit(const std::string& _str, const bool& _blocking_mode){
	int32_t bytes_to_send = _str.length();
	uint32_t pos = 0;
	uint8_t retries = 0;
	bool success = false;
	success = false;
	while(bytes_to_send > 0){
		std::memset(tx_buffer, '\0', uart_dma_tx_buffer_size);
		_str.copy((char*)tx_buffer, SmallerOfTwo(bytes_to_send, uart_dma_tx_buffer_size), pos);
		if (dev_valid){
			if (HAL_UART_Transmit_DMA(uart_handle, tx_buffer, SmallerOfTwo(bytes_to_send, uart_dma_tx_buffer_size)) == HAL_OK){
				if (_blocking_mode)	ulTaskNotifyTake( xArrayIndex, osWaitForever);
				bytes_to_send -= uart_dma_tx_buffer_size;
				pos += uart_dma_tx_buffer_size;
				success = true;
				retries = 0;
			}
			else{
				retries++;
				success = false;
			}
			if (retries > 20) break;
		}
	}
	return success;
}

IncomingMessage HAL_UART_DMA_MsgBroker::getIncoming(MsgParser *_parser)
{
	if (_parser != nullptr) return _parser->parseIncoming(rx_buffer, msg_len);
	else if( parser != nullptr)	return parser->parseIncoming(rx_buffer, msg_len);
	return {};
}

bool HAL_UART_DMA_MsgBroker::setMsgLength(const size_t& _msg_len){
	if (_msg_len < 0) return false; 
	if (_msg_len >= 0) msg_len = _msg_len;
	return true;
}
size_t& HAL_UART_DMA_MsgBroker::getMsgLength(){
	return msg_len;
}


