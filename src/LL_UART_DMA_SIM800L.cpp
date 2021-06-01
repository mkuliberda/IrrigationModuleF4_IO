#include "ll_uart_dma_sim800l.h"
#include "utilities.h"
#include <algorithm>

extern uint8_t usart3_rx_dma_buffer[64];

std::vector<std::string> SIM800LConfiguration::getConfiguration (){
  return configuration;
}

bool LL_UART_DMA_SIM800L::configure(void *_config) {
  if(_config == nullptr) return false;
  config_handle = static_cast<SIM800LConfiguration*>(_config);
  auto config_vec = config_handle->getConfiguration();
  for_each (config_vec.begin(), config_vec.end(), [&](const std::string& _val){ transmit(_val + "\r"); } );
  return true;   
}
bool LL_UART_DMA_SIM800L::isReady() {
    return false;    
}

uint32_t LL_UART_DMA_SIM800L::getSignal() {
    return 0;    
}

RegistrationStatus LL_UART_DMA_SIM800L::getRegistrationStatus() {
    return RegistrationStatus::Unknown;    
}

void LL_UART_DMA_SIM800L::reset() {
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

bool LL_UART_DMA_SIM800L::sendSms(const std::string_view& _mobile_number, const std::string_view& text) {
  transmit(at_command[SIM800L_Command::SET_SMS_TEXT_MODE]);
  read();
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
    return false;
}

std::string LL_UART_DMA_SIM800L::readSms(const uint32_t& _index) {
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
   return "";
}

std::string LL_UART_DMA_SIM800L::getSmsSender(const uint32_t& _index) {
    /*
      _buffer=readSms(index);
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
   return "";
}

bool LL_UART_DMA_SIM800L::deleteAllSms() {
  /*SIM.print(F("at+cmgda=\"del all\"\n\r"));
  _buffer=_readSerial();
  if (_buffer.indexOf("OK")!=-1) {return true;}else {return false;}*/
  return false;
}

bool LL_UART_DMA_SIM800L::read() {
  	if (!periph_valid) return false;

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

bool LL_UART_DMA_SIM800L::transmit(const std::string& _str){
	size_t bytes_to_send = _str.length();
	if (!periph_valid) {
		return false;
	}
	for(size_t byte_pos=0; byte_pos < bytes_to_send; byte_pos++) {
		LL_USART_TransmitData8(uart_handle, _str.at(byte_pos));
		while (!LL_USART_IsActiveFlag_TXE(uart_handle));
	}
	while (!LL_USART_IsActiveFlag_TC(uart_handle));
	return true;
}


std::string LL_UART_DMA_SIM800L::processBuffer(const void* data, size_t len) {
    const uint8_t* d = (uint8_t*)data;	
    /*
     * This function is called on DMA TC and HT events, aswell as on UART IDLE (if enabled) line event.
     */

    std::string buffer{};
    buffer.reserve(len);
    for (; len > 0; --len, ++d) {
      buffer.push_back(*d);
		//rx_buffer.push_back(*d);
    }
    return buffer;
	//parser->parseString(rx_buffer);
	//rx_buffer.clear();
}
