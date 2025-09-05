#include "radio_interface.h"
#include <RadioLib.h>

// implementation for RFM98
// NSS pin:   10
// DIO0 pin:  2
// RESET pin: 9
// DIO1 pin:  3
static RFM98 radio = new Module(RADIO_RFM_NSS_PIN, RADIO_RFM_DIO0_PIN, RADIO_RFM_NRST_PIN, RADIO_RFM_DIO1_PIN); 
static int res = RADIOLIB_ERR_NONE; 

// flags 
static volatile bool operation_done_flag = false; 
static volatile bool detected_flag = false; 

// isrs 
static void raiseOperationDoneFlag(){
  operation_done_flag = true; 
}
static bool popOperationDoneFlag(){
  if(operation_done_flag){
    operation_done_flag = false;
    return true; 
  } else {
    return false; 
  }
}

static void raiseDetectedFlag(){
  detected_flag = true; 
}
static bool popDetectedFlag(){
  if(detected_flag){
    detected_flag = false; 
    return true; 
  } else {
    return false; 
  }
}

// status checkers (can be functions or just flags set from interrupt)
// they should clean up after they return the status 
bool serial_input(){
  return Serial.available() > 0; 
} 

bool scan_timeout(){
  return popOperationDoneFlag(); 
} 

bool activity_detected(){
  return popDetectedFlag(); 
}

bool receive_done(){
  return popOperationDoneFlag();  
}

bool transmit_done(){
  return popOperationDoneFlag(); 
} 

bool receive_timeout(){
  return false;
}

bool transmit_timeout(){
  return false; 
} 

// operations 
void initRadio(){
  res = radio.begin(RADIO_FREQ, RADIO_BW_FAST, RADIO_SF_FAST, RADIO_CR_FAST, RADIO_SYNC_WORD, RADIO_TRANSMIT_POWER, RADIO_PREAMBLE_LEN, RADIO_RFM_GAIN);

  if(res != RADIOLIB_ERR_NONE){
    Serial.print(F("radio.begin failed, code "));
    Serial.println(res);
    while (true)
      ;
  }
  
  radio.setDio0Action(raiseOperationDoneFlag, RISING);
  radio.setDio1Action(raiseDetectedFlag, RISING);
}

void startActivityDetection(){
  res = radio.startChannelScan(); 

  if(res != RADIOLIB_ERR_NONE){
    Serial.print(F("radio.startChannelScan failed, code "));
    Serial.println(res);
    while (true)
      ;
  }
}

void startReceive(){
  res = radio.startReceive(); 

  if(res != RADIOLIB_ERR_NONE){
    Serial.print(F("radio.startReceive failed, code "));
    Serial.println(res);
    while (true)
      ;
  }
} 

void receiveAndForward(){
  size_t len = radio.getPacketLength(); 
  uint8_t buf[len]; 
  res = radio.readData(buf, len); 

  if(res == RADIOLIB_ERR_NONE){
    Serial.write(buf, len); 
  }
} 

void decodeAndTransmit(){
  // decode - TODO: make this smarter 
  uint8_t buf[256]; 
  size_t len = 0; 
  
  // while there are still bytes to send
  while(Serial.available() > 0 && len < 256){
    // read them into the buffer
    buf[len++] = Serial.read(); 
  }
  // transmit the entire buffer
  radio.startTransmit(buf, len); 

} 

void transmitCleanUp(){
  radio.finishTransmit(); 
}

void adaptRadio(){
  // use with decode 
} 

