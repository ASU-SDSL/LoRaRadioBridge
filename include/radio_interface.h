#ifndef RADIO_INTERFACE_H
#define RADIO_INTERFACE_H

#include "Arduino.h"

// status checkers (can be functions or just flags set from interrupt)
// they should clean up after they return the status 
/**
 * @brief Returns if there is data waiting on the serial buffer
 * 
 * @return true 
 * @return false 
 */
bool serial_input();

/**
 * @brief Returns true on Channel Activity Detection operation timeout 
 * when an activity is NOT detected
 * 
 * @return true 
 * @return false 
 */
bool scan_timeout(); 

/**
 * @brief Returns true on Channel Activity Detection operation timeout 
 * when an activity IS detected 
 * 
 * @return true 
 * @return false 
 */
bool activity_detected();

/**
 * @brief Returns true on receive operation finish 
 * 
 * @return true 
 * @return false 
 */
bool receive_done();

/**
 * @brief Returns true on transmit operation finish 
 * 
 * @return true 
 * @return false 
 */
bool transmit_done(); 

/**
 * @brief Returns true on receive operation timeout 
 * 
 * @return true 
 * @return false 
 */
bool receive_timeout();

/**
 * @brief Returns true on transmit operation timeout 
 * 
 * @return true 
 * @return false 
 */
bool transmit_timeout(); 

// operations 
/**
 * @brief Initializes radio 
 * 
 */
void initRadio(); 

/**
 * @brief Starts CAD scanning
 * 
 */
void startActivityDetection(); 

/**
 * @brief Starts receive 
 * 
 */
void startReceive(); 

/**
 * @brief Completes receive and adds received data to Serial out buffer
 * 
 */
void receiveAndForward(); 

/**
 * @brief Reads data from Serial in buffer and transmits it, optionally 
 * decoding it for use later in adaptRadio() 
 * 
 */
void decodeAndTransmit(); 

/**
 * @brief Cleans Up after transmission, radio should be ready to go into 
 * any state after this function 
 * 
 */
void transmitCleanUp();

/**
 * @brief Adapts to follow decoded message if needed 
 * 
 */
void adaptRadio();  

#endif 