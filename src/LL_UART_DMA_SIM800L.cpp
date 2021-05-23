#include "ll_uart_dma_sim800l.h"

std::string LL_UART_DMA_SIM800L::read() {
    return "";     
}

bool LL_UART_DMA_SIM800L::configure() {
    return false;    
}
bool LL_UART_DMA_SIM800L::isReady() {
    return false;    
}

uint32_t LL_UART_DMA_SIM800L::getSignal() {
    return 0;    
}

SIM800L_RegistrationStatus LL_UART_DMA_SIM800L::getRegistrationStatus() {
    return SIM800L_RegistrationStatus::UNKNOWN;    
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

bool LL_UART_DMA_SIM800L::assignPeripheral(void *_periph_handle) {
	if (_periph_handle == nullptr) return false;
	uart_handle = static_cast<UART_HandleTypeDef*>(_periph_handle);
	return periph_valid = true;
   
}
