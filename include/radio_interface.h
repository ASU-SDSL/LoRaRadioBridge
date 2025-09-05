#ifndef RADIO_INTERFACE_H
#define RADIO_INTERFACE_H

#include "Arduino.h"

// status checkers (can be functions or just flags set from interrupt)
// they should clean up after they return the status 
bool serial_input(); 
bool scan_timeout(); 
bool activity_detected();
bool receive_done();
bool transmit_done(); 
bool receive_timeout();
bool transmit_timeout(); 

// operations 
void startActivityDetection(); 
void startReceive(); 
void receiveAndForward(); 
void decodeAndTransmit(); 
void transmitCleanUp();
void adaptRadio();  

#endif 