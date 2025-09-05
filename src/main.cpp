
#include "Arduino.h"
#include <RadioLib.h>
#include "radio_interface.h"

// states
typedef enum : uint8_t {
  IDLE,
  DETECTING, 
  DECODE, 
  TRANSMITTING,
  ADAPT,
  RECEIVING,
  FORWARD
} sm_t; 

sm_t state = IDLE; 
int res = 0;  


void setup(){
  // the init state
  Serial.begin(115200); 
  while(!Serial)
    ;

  initRadio(); 
}

void loop(){
  sm_t next_state = state; 

  switch(state){
    
    case IDLE:
      if(serial_input()){
        next_state = DECODE; 
      } else {
        startActivityDetection(); 
        next_state = DETECTING; 
      }
      break;

    case DETECTING:
      if(scan_timeout()){
        next_state = IDLE; 
      } else if(activity_detected()){
        startReceive(); 
        next_state = RECEIVING; 
      }
      break; 

    case RECEIVING:
      if(receive_done()){
        next_state = FORWARD; 
      } else if(receive_timeout()){
        next_state = IDLE; 
      } else {
        next_state = RECEIVING; 
      }
      break;

    case FORWARD:
      receiveAndForward(); 
      next_state = IDLE;       
      break; 

    case DECODE:
      decodeAndTransmit(); 
      next_state = TRANSMITTING; 
      break;

    case TRANSMITTING:
      if(transmit_done()){
        transmitCleanUp(); 
        next_state = ADAPT; 
      } else if(transmit_timeout()){
        transmitCleanUp(); 
        next_state = IDLE; 
      } else {
        next_state = TRANSMITTING; 
      }
      break;

    case ADAPT:
      adaptRadio(); 
      next_state = IDLE; 
      break;
    
    default:
      Serial.print("Bad state: "); Serial.println(state); 
      break; 

  }

  state = next_state; 

}