#ifndef RADIO_INTERFACE_H
#define RADIO_INTERFACE_H

#include "Arduino.h"

// constants and settings
// RFM pinout
#define RADIO_RFM_NSS_PIN 10
#define RADIO_RFM_DIO0_PIN 2
#define RADIO_RFM_NRST_PIN 9
#define RADIO_RFM_DIO1_PIN 3

// Consistent radio settings: 
#define RADIO_FREQ 434.0
#define RADIO_SYNC_WORD 18
#define RADIO_PREAMBLE_LEN 8
#define RADIO_RFM_GAIN 0      // (auto)
#define RADIO_TRANSMIT_POWER 21

// LoRa Modes:
// fast mode (~4 kbps)
#define RADIO_BW_FAST 62.5
#define RADIO_SF_FAST 6
#define RADIO_CR_FAST 5

// safe mode (~400 bps)
#define RADIO_BW_SAFE 62.5
#define RADIO_SF_SAFE 10
#define RADIO_CR_SAFE 5

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