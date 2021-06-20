#include "ll_uart_dma_sim800l.h"
#include "utilities.h"
#include <algorithm>
#include <memory>
#include "cmsis_os.h"
#include <sstream>

extern uint8_t usart3_rx_dma_buffer[64];
const UBaseType_t xTaskCount = 1;

ProcessResult LL_UART_DMA_SIM800L::configure(void *_config) {
  if(_config != nullptr) {
    config_handle = static_cast<SIM800LConfiguration*>(_config);
  }
  if(config_handle == nullptr) return ProcessResult::ERROR;

  for (const auto &token: config_handle->getConfiguration()){
    transmit(token);
    if (read() == ProcessResult::ERROR) return ProcessResult::ERROR;
  }
  return ProcessResult::OK;   
}
bool LL_UART_DMA_SIM800L::isReady() {
  ModuleState state = getState();
  if(state != ModuleState::Invalid && state != ModuleState::Unknown && periph_valid) return true;
  return false;    
}

ModuleState LL_UART_DMA_SIM800L::getState(){

    return state;
}

uint32_t LL_UART_DMA_SIM800L::getSignal() {
    return 0;    
}

RegistrationStatus LL_UART_DMA_SIM800L::getNetworkRegistrationStatus() {
  transmit(at_command[SIM800L_Attention::GET_NETWORK_REG_STATUS]);
  std::string answer = readAnswer();
  std::vector<std::string> tokens = tokenizeString(answer, ",: \r\n");
  if(tokens.size() == 4){
    uint8_t status_code = atoi(tokens.at(3).c_str());
    if(status_code == 1 || status_code == 5) return RegistrationStatus::Registered;
    if(status_code == 0 || status_code == 3) return RegistrationStatus::Error;
    if(status_code == 2 || status_code == 4) return RegistrationStatus::Unknown;
  }
  return RegistrationStatus::Unknown;    
}

ProcessResult LL_UART_DMA_SIM800L::reset() {
  return ProcessResult::ERROR;
    /*
      #if (LED)
    digitalWrite(LED_PIN,1);
  #endif 
  digitalWrite(RESET_PIN,1);
  delay(1000);
  digitalWrite(RESET_PIN,0);
  delay(1000);
  // wait for the module response

  SIM.print(F("AT\r\n"));
  while (_readSerial().indexOf("OK")==-1 ){
    SIM.print(F("AT\r\n"));
  }
  
  //wait for sms ready
  while (_readSerial().indexOf("SMS")==-1 ){
  }
  #if (LED)
    digitalWrite(LED_PIN,0);
  #endif 
    */
}

ProcessResult LL_UART_DMA_SIM800L::sendMessage(const std::string_view& _mobile_number, const std::string_view& text) {
  transmit(at_command[SIM800L_Attention::SET_SMS_MODE_TEXT]);
  if (ProcessResult::ERROR == read()){
    return ProcessResult::ERROR;
  }

  //std::ostringstream oss;
  //oss<<at_command[SIM800L_Attention::NEW_SMS_BEGIN]<<_mobile_number<<"\"\r";
  //transmit(oss.str());
  transmit(at_command[SIM800L_Attention::NEW_SMS_BEGIN] + patch::to_string(_mobile_number) + "\"\r");
  state = ModuleState::NewSms;
  if (ProcessResult::ERROR == read()){
    state = ModuleState::Invalid;
    return ProcessResult::ERROR;
  }

  //oss.str("");
  //oss.clear();
  //oss<<text<<"\r";
  //transmit(oss.str());
  transmit(patch::to_string(text) + "\r");
  if (ProcessResult::ERROR == read()){
    state = ModuleState::Invalid;
    return ProcessResult::ERROR;
  }
  
  //oss.str("");
  //oss.clear();
  //oss<<"CMGS:"<<_mobile_number;
  transmit((char)26);
  state = ModuleState::SendingSms;
  if (ProcessResult::ERROR == read("CMGS:" + patch::to_string(_mobile_number), "ERROR")){
    state = ModuleState::Invalid;
    return ProcessResult::ERROR;
  }
  state = ModuleState::Idle;

  return ProcessResult::OK;

/*        SIM.print (F("AT+CMGF=1\r")); //set sms to text mode  
    _buffer=_readSerial();
    SIM.print (F("AT+CMGS=\""));  // command to send sms
    SIM.print (number);           
    SIM.print(F("\"\r"));       
    _buffer=_readSerial(); 
    SIM.print (text);
    SIM.print ("\r"); 
	//change delay 100 to readserial	
    _buffer=_readSerial();
    SIM.print((char)26);
    _buffer=_readSerial();
    //expect CMGS:xxx   , where xxx is a number,for the sending sms.
    if (((_buffer.indexOf("CMGS") ) != -1 ) ){
      return true;
    }
    else {
      return false;
    }*/
}

std::string LL_UART_DMA_SIM800L::readMessage(const uint32_t& _index) {
  transmit(at_command[SIM800L_Attention::SET_SMS_MODE_TEXT]);
  if (ProcessResult::ERROR == read()){
    return "ERROR";
  }
  
    /*
  SIM.print (F("AT+CMGF=1\r")); 
  if (( _readSerial().indexOf("ER")) ==-1) {
    SIM.print (F("AT+CMGR="));
    SIM.print (index);
    SIM.print("\r");
    _buffer=_readSerial();
    if (_buffer.indexOf("CMGR:")!=-1){
      return _buffer;
    }
    else return "";    
    }
  else
    return "";
  }
    */
   return "Not implemented yet";
}

std::string LL_UART_DMA_SIM800L::getMessageOrigin(const uint32_t& _index) {
    /*
      _buffer=readMessage(index);
  Serial.println(_buffer.length());
  if (_buffer.length() > 10) //avoid empty sms
  {
    uint8_t _idx1=_buffer.indexOf("+CMGR:");
    _idx1=_buffer.indexOf("\",\"",_idx1+1);
    return _buffer.substring(_idx1+3,_buffer.indexOf("\",\"",_idx1+4));
  }else{
    return "";
  }
    */
   return "Not implemented yet";
}

ProcessResult LL_UART_DMA_SIM800L::deleteAllMessages() {
  /*SIM.print(F("at+cmgda=\"del all\"\n\r"));
  _buffer=_readSerial();
  if (_buffer.indexOf("OK")!=-1) {return true;}else {return false;}*/
  return ProcessResult::ERROR;
}

/*ProcessResult LL_UART_DMA_SIM800L::read(){
  ProcessResult result{ProcessResult::OK};
  bool ok{};
  bool error{};
  do {
    ulTaskNotifyTake(xTaskCount, osWaitForever);
    processRxEvent();
    ok = rx_buffer.find("OK\r\n") != std::string::npos;
    error = rx_buffer.find("ERROR") != std::string::npos;
    if (error) result = ProcessResult::ERROR;
  } while(ok == false && error == false);
  rx_buffer.clear();
  return result;
}*/

ProcessResult LL_UART_DMA_SIM800L::read(const std::string& _expected_ok, const std::string& _expected_error){
  ProcessResult result{ProcessResult::OK};
  bool ok{};
  bool error{};
  do {
    ulTaskNotifyTake(xTaskCount, osWaitForever);
    processRxEvent();
    ok = rx_buffer.find(_expected_ok) != std::string::npos;
    error = rx_buffer.find(_expected_error) != std::string::npos;
    if (error) result = ProcessResult::ERROR;
  } while(ok == false && error == false);
  rx_buffer.clear();
  return result;
}

std::string LL_UART_DMA_SIM800L::readAnswer(const std::string& _expected_ok, const std::string& _expected_error){
  bool ok{};
  bool error{};
  do {
    ulTaskNotifyTake(xTaskCount, osWaitForever);
    processRxEvent();
    ok = rx_buffer.find(_expected_ok) != std::string::npos;
    error = rx_buffer.find(_expected_error) != std::string::npos;
  } while(ok == false && error == false);

  std::string result = std::move(rx_buffer);
  rx_buffer.clear(); //TODO: check if this really moves the string and if clear() is necessary
  return result;
}


void LL_UART_DMA_SIM800L::processRxEvent() {

    size_t pos;

    /* Calculate current position in buffer */
    pos = ArrayLength(usart3_rx_dma_buffer) - LL_DMA_GetDataLength(DMA1, LL_DMA_STREAM_1);
    if (pos != old_pos) {                       /* Check change in received data */
        if (pos > old_pos) {                    /* Current position is over previous one */
            /* We are in "linear" mode */
            /* Process data directly by subtracting "pointers" */
            accumulateRxBuffer(&usart3_rx_dma_buffer[old_pos], pos - old_pos);
        } else {
            /* We are in "overflow" mode */
            /* First process data to the end of buffer */
            accumulateRxBuffer(&usart3_rx_dma_buffer[old_pos], ArrayLength(usart3_rx_dma_buffer) - old_pos);
            /* Check and continue with beginning of buffer */
            if (pos > 0) {
                accumulateRxBuffer(&usart3_rx_dma_buffer[0], pos);
            }
        }
        old_pos = pos;/* Save current position as old */
    }
}

ProcessResult LL_UART_DMA_SIM800L::transmit(const std::string& _str){
	if (!periph_valid) {
		return ProcessResult::ERROR;
	}

  size_t bytes_to_send = _str.length();
	for(size_t byte_pos=0; byte_pos < bytes_to_send; byte_pos++) {
		LL_USART_TransmitData8(uart_handle, _str.at(byte_pos));
		while (!LL_USART_IsActiveFlag_TXE(uart_handle));
	}
	while (!LL_USART_IsActiveFlag_TC(uart_handle));
	return ProcessResult::OK;
}

ProcessResult LL_UART_DMA_SIM800L::transmit(const char& ch){
	if (!periph_valid) {
		return ProcessResult::ERROR;
	}
	LL_USART_TransmitData8(uart_handle, ch);
	while (!LL_USART_IsActiveFlag_TXE(uart_handle));
	while (!LL_USART_IsActiveFlag_TC(uart_handle));
	return ProcessResult::OK;
}


void LL_UART_DMA_SIM800L::accumulateRxBuffer(const void* data, size_t len) {
    const uint8_t* d = (uint8_t*)data;	
    /*
     * This function is called on DMA TC and HT events, aswell as on UART IDLE (if enabled) line event.
     */

    for (; len > 0; --len, ++d) {
      rx_buffer.push_back(*d);
    }
}

std::vector<std::string> LL_UART_DMA_SIM800L::tokenizeString(const std::string &s, const std::string &delim) {
    static bool dict[256] = { false};

    std::vector<std::string> res;
    for (auto i = 0; i < delim.size(); ++i) {      
        dict[delim[i]] = true;
    }

    std::string token("");
    for (auto &i : s) {
        if (dict[i]) {
            if (!token.empty()) {
                res.push_back(token);
                token.clear();
            }           
        }
        else {
            token += i;
        }
    }
    if (!token.empty()) {
        res.push_back(token);
    }
    return res;
}
