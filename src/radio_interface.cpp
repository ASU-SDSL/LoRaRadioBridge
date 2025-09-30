/**
 * @file radio_interface.cpp
 * @author Tyler Nielsen
 * @brief Implementation of the radio_interface.h functions for the RFM98PW
 * @version 0.1
 * @date 2025-09-04
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "radio_interface.h"

#include <RadioLib.h>

#include <vector>

// constants and settings
// RFM pinout
#define RADIO_RFM_NSS_PIN 10
#define RADIO_RFM_DIO0_PIN 2
#define RADIO_RFM_NRST_PIN 9
#define RADIO_RFM_DIO1_PIN 3

// Consistent radio settings:
#define RADIO_FREQ 437.400
#define RADIO_SYNC_WORD 18
#define RADIO_PREAMBLE_LEN 8
#define RADIO_RFM_GAIN 0  // (auto)
// update with 1W macro if needed (that may be a bs number bc RabioLib
// implements the base module not the 1W version)
#define RADIO_TRANSMIT_POWER 10

#define TRANSMIT_TIMEOUT_US (1000 * 1000)  // timeout after 1s (1,000,000 us)
#define RECEIVE_TIMEOUT_US (1000 * 1000)   // timeout after 1s (1,000,000 us)

// LoRa Modes:
// fast mode (~4 kbps)
#define RADIO_BW 62.5
#define RADIO_SF_FAST 6
#define RADIO_CR 5

// safe mode (~400 bps)
#define RADIO_SF_SAFE 10

// shared variables
static RFM98 radio = new Module(RADIO_RFM_NSS_PIN, RADIO_RFM_DIO0_PIN,
                                RADIO_RFM_NRST_PIN, RADIO_RFM_DIO1_PIN);
static int res = RADIOLIB_ERR_NONE;
static uint64_t timestamp = 0;

// timing
static uint64_t now() { return to_us_since_boot(get_absolute_time()); }

// flags
static volatile bool operation_done_flag = false;
static volatile bool detected_flag = false;

// isrs
static void raiseOperationDoneFlag() { operation_done_flag = true; }
static bool popOperationDoneFlag() {
  if (operation_done_flag) {
    operation_done_flag = false;
    return true;
  } else {
    return false;
  }
}

static void raiseDetectedFlag() { detected_flag = true; }
static bool popDetectedFlag() {
  if (detected_flag) {
    detected_flag = false;
    return true;
  } else {
    return false;
  }
}

// status checkers (can be functions or just flags set from interrupt)
// they should clean up after they return the status
bool serial_input() { return Serial.available() > 0; }

bool scan_timeout() { return popOperationDoneFlag(); }

bool activity_detected() { return popDetectedFlag(); }

bool receive_done() { return popOperationDoneFlag(); }

bool transmit_done() { return popOperationDoneFlag(); }

bool receive_timeout() { return now() - timestamp > RECEIVE_TIMEOUT_US; }

bool transmit_timeout() { return now() - timestamp > TRANSMIT_TIMEOUT_US; }

// operations
void initRadio() {
  res = radio.begin(RADIO_FREQ, RADIO_BW, RADIO_SF_SAFE, RADIO_CR,
                    RADIO_SYNC_WORD, RADIO_TRANSMIT_POWER, RADIO_PREAMBLE_LEN,
                    RADIO_RFM_GAIN);

  if (res != RADIOLIB_ERR_NONE) {
    debug_print(F("radio.begin failed, code "));
    debug_println(res);
    while (true)
      ;
  }

  radio.setDio0Action(raiseOperationDoneFlag, RISING);
  radio.setDio1Action(raiseDetectedFlag, RISING);
}

void startActivityDetection() {
  res = radio.startChannelScan();

  if (res != RADIOLIB_ERR_NONE) {
    debug_print(F("radio.startChannelScan failed, code "));
    debug_println(res);
    while (true)
      ;
  }
}

void startReceive() {
  res = radio.startReceive();

  if (res != RADIOLIB_ERR_NONE) {
    debug_print(F("radio.startReceive failed, code "));
    debug_println(res);
    while (true)
      ;
  }

  // start timeout timer
  timestamp = now();
}

void receiveAndForward() {
  size_t len = radio.getPacketLength();
  std::vector<uint8_t> buf(len);
  res = radio.readData(buf.data(), len);

  if (res == RADIOLIB_ERR_NONE) {
    Serial.write(buf.data(), len);
  }
}

void decodeAndTransmit() {
  // decode - TODO: make this smarter
  uint8_t buf[256];
  size_t len = 0;

  // while there are still bytes to send
  while (Serial.available() > 0 && len < 256) {
    // read them into the buffer
    buf[len++] = Serial.read();
  }
  // transmit the entire buffer
  radio.startTransmit(buf, len);

  // start timeout timer
  timestamp = now();
}

void transmitCleanUp() {
  // clean up needed after every transmit
  radio.finishTransmit();
}

void adaptRadio() {
  // use with decode
}
