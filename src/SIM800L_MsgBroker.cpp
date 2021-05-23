#include "SIM800L_MsgBroker.h"
#include "utilities.h"
#include <string>
#include <cstring>
#include "cmsis_os.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_dma.h"
#include "utilities.h"
#include "usart.h"

extern const UBaseType_t xArrayIndex;

bool SIM800L_MsgBroker::assignDevice(void *_dev_handle)
{
	if (_dev_handle == nullptr) return false;
	dev_handle = static_cast<SIM800L*>(_dev_handle);
	return dev_valid = true;
}

bool SIM800L_MsgBroker::assignPeripheral(void *_periph_handle)
{
	if (_periph_handle == nullptr) return false;
	uart_handle = static_cast<UART_HandleTypeDef*>(_periph_handle);
	return periph_valid = true;
}

bool SIM800L_MsgBroker::sendMsg(const ExternalObject& _recipient, const InternalObject& _publisher, const std::string& _msg, const bool& _wait_until_cplt, Encoder *_encoder)
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

bool SIM800L_MsgBroker::publishData(const ExternalObject& _recipient, const InternalObject& _publisher, std::unordered_map<std::string, int32_t> _values, const bool& _wait_until_cplt, Encoder *_encoder)
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

bool SIM800L_MsgBroker::requestData(const ExternalObject& _recipient, const InternalObject& _publisher, const std::string& _data_key, const std::string& _data_type, const bool& _wait_until_cplt, Encoder *_encoder)
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

 bool SIM800L_MsgBroker::setDefaultParser(MsgParser *_parser){
	if (_parser == nullptr) return false;
	delete this->parser;
 	parser= _parser;
 	return true;
 	
 }
 bool SIM800L_MsgBroker::setDefaultEncoder(Encoder*_encoder){
	 if (_encoder == nullptr) return false;
	 delete this->encoder;
	 encoder = _encoder;
	 return true;
 }

void SIM800L_MsgBroker::setExternalAddresses(std::unordered_map<ExternalObject_t, std::string> *_addresses)
{
	ext_address_map = _addresses;
}
void SIM800L_MsgBroker::setInternalAddresses(std::unordered_map<InternalObject_t, std::string> *_addresses)
{
	int_address_map = _addresses;
}

bool SIM800L_MsgBroker::read(){

	if (!periph_valid){
		return false;
	}
    //static size_t old_pos;
    size_t pos;

    /* Calculate current position in buffer */
    pos = ArrayLength(usart3_rx_dma_buffer) - LL_DMA_GetDataLength(DMA1, LL_DMA_STREAM_1);
    if (pos != old_pos) {                       /* Check change in received data */
        if (pos > old_pos) {                    /* Current position is over previous one */
            /* We are in "linear" mode */
            /* Process data directly by subtracting "pointers" */
            processBuffer(&usart3_rx_dma_buffer[old_pos], pos - old_pos);
        } else {
            /* We are in "overflow" mode */
            /* First process data to the end of buffer */
            processBuffer(&usart3_rx_dma_buffer[old_pos], ArrayLength(usart3_rx_dma_buffer) - old_pos);
            /* Check and continue with beginning of buffer */
            if (pos > 0) {
                processBuffer(&usart3_rx_dma_buffer[0], pos);
            }
        }
        old_pos = pos;                          /* Save current position as old */
    }


	//ulTaskNotifyTake( xArrayIndex, osWaitForever);
	return true;
}

bool SIM800L_MsgBroker::transmit(const std::string& _str, const bool& _blocking_mode){
	size_t bytes_to_send = _str.length();
	if (!periph_valid) {
		return false;
	}
	for(size_t byte_pos=0; byte_pos < bytes_to_send; byte_pos++) {
		LL_USART_TransmitData8(uart_handle->Instance, _str.at(byte_pos));
		while (!LL_USART_IsActiveFlag_TXE(uart_handle->Instance));
	}
	while (!LL_USART_IsActiveFlag_TC(uart_handle->Instance));
	return true;
}


void SIM800L_MsgBroker::processBuffer(const void* data, size_t len) {
    const uint8_t* d = (uint8_t*)data;	
    /*
     * This function is called on DMA TC and HT events, aswell as on UART IDLE (if enabled) line event.
     * 
     * For the sake of this example, function does a loop-back data over UART in polling mode.
     * Check ringbuff RX-based example for implementation with TX & RX DMA transfer.
     */

	
    for (; len > 0; --len, ++d) {
		rx_buffer.push_back(*d);
    }
	parser->parseString(rx_buffer);
	rx_buffer.clear();
}

IncomingMessage SIM800L_MsgBroker::getIncoming(MsgParser *_parser)
{
	/*if (_parser != nullptr) return _parser->parseIncoming(usart3_rx_dma_buffer, msg_len);
	else if( parser != nullptr)	return parser->parseIncoming(usart3_rx_dma_buffer, msg_len);*/
	return {};
}

bool SIM800L_MsgBroker::setMsgLength(const size_t& _msg_len){
	return false;
}
size_t& SIM800L_MsgBroker::getMsgLength(){
	return msg_len;
}



