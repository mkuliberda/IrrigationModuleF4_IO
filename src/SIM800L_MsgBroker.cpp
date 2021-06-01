#include "SIM800L_MsgBroker.h"
#include "utilities.h"
#include <string>
#include <cstring>
#include "cmsis_os.h"
#include "utilities.h"
#include "usart.h"

extern const UBaseType_t xArrayIndex;

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
		//return transmit(_encoder->str(), _wait_until_cplt);
	}
	if (encoder != nullptr)
	{
		hdr.accept(*encoder);
		msg.accept(*encoder);
		end.accept(*encoder);
		//return transmit(encoder->str(), _wait_until_cplt);
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
		//return transmit(_encoder->str(), _wait_until_cplt);
	}
	else if(encoder != nullptr)
	{
		for (auto &item : msg_items) item->accept(*encoder);
		//return transmit(encoder->str(), _wait_until_cplt);
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
		//return transmit(_encoder->str(), _wait_until_cplt);
	}
	if (encoder != nullptr)
	{
		hdr.accept(*encoder);
		msg.accept(*encoder);
		end.accept(*encoder);
		//return transmit(encoder->str(), _wait_until_cplt);
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
	return false;
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
size_t SIM800L_MsgBroker::getMsgLength() const {
	return msg_len;
}



