#include "radio_interface.h"

// status checkers (can be functions or just flags set from interrupt)
// they should clean up after they return the status 
bool serial_input(){
  return Serial.available() > 0; 
} 

bool scan_timeout(){
  return false; 
} 

bool activity_detected(){
  return false; 
}

bool receive_done(){
  return false; 
}

bool transmit_done(){
  return false; 
} 

bool receive_timeout(){
  return false; 
}

bool transmit_timeout(){
  return false; 
} 

// operations 
void startActivityDetection(){

}

void startReceive(){

} 

void receiveAndForward(){

} 

void decodeAndTransmit(){

} 

void transmitCleanUp(){

}

void adaptRadio(){

} 
 
