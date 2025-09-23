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
#define RADIO_FREQ 434.0
#define RADIO_SYNC_WORD 18
#define RADIO_PREAMBLE_LEN 8
#define RADIO_RFM_GAIN 0  // (auto)
#define RADIO_TRANSMIT_POWER 21
#define COMMAND_SYNC_BYTES "\x35\x2E\xF8\x53"

// LoRa Modes:
// fast mode (~4 kbps)
#define RADIO_BW_FAST 62.5
#define RADIO_SF_FAST 6
#define RADIO_CR_FAST 5

// safe mode (~400 bps)
#define RADIO_BW_SAFE 62.5
#define RADIO_SF_SAFE 10
#define RADIO_CR_SAFE 5

// Mode setting in command packet
#define RADIO_SAFE_MODE 0
#define RADIO_FAST_MODE 1

// shared variables
static RFM98 radio = new Module(RADIO_RFM_NSS_PIN, RADIO_RFM_DIO0_PIN,
                                RADIO_RFM_NRST_PIN, RADIO_RFM_DIO1_PIN);
static int res = RADIOLIB_ERR_NONE;

// radio begin modes
static int radio_begin_safe() {
  return radio.begin(RADIO_FREQ, RADIO_BW_SAFE, RADIO_SF_SAFE, RADIO_CR_SAFE,
                     RADIO_SYNC_WORD, RADIO_TRANSMIT_POWER, RADIO_PREAMBLE_LEN,
                     RADIO_RFM_GAIN);
}
static int radio_begin_fast() {
  return radio.begin(RADIO_FREQ, RADIO_BW_FAST, RADIO_SF_FAST, RADIO_CR_FAST,
                     RADIO_SYNC_WORD, RADIO_TRANSMIT_POWER, RADIO_PREAMBLE_LEN,
                     RADIO_RFM_GAIN);
}

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

bool receive_timeout() { return false; }

bool transmit_timeout() { return false; }

// operations
void initRadio() {
  res = radio_begin_safe();

  if (res != RADIOLIB_ERR_NONE) {
    Serial.print(F("radio.begin failed, code "));
    Serial.println(res);
    while (true);
  }

  radio.setDio0Action(raiseOperationDoneFlag, RISING);
  radio.setDio1Action(raiseDetectedFlag, RISING);
}

void startActivityDetection() {
  res = radio.startChannelScan();

  if (res != RADIOLIB_ERR_NONE) {
    Serial.print(F("radio.startChannelScan failed, code "));
    Serial.println(res);
    while (true);
  }
}

void startReceive() {
  res = radio.startReceive();

  if (res != RADIOLIB_ERR_NONE) {
    Serial.print(F("radio.startReceive failed, code "));
    Serial.println(res);
    while (true);
  }
}

void receiveAndForward() {
  size_t len = radio.getPacketLength();
  std::vector<uint8_t> buf(len);
  res = radio.readData(buf.data(), len);

  if (res == RADIOLIB_ERR_NONE) {
    Serial.write(buf.data(), len);
  }
}

#define SPACEPACKET_ENCODED_HEADER_SIZE 6

struct spacepacket_header {
  uint8_t version;
  bool type;
  bool secondary_header_flag;
  uint16_t apid;
  uint8_t sequence_flag;
  uint16_t packet_sequence_count;
  uint16_t packet_length;
} typedef spacepacket_header_t;

/*
    Decodes a byte buffer into a SpacePacket header struct
    Argument encoded_buf must be at least 6 bytes large
    Returns 0 on success, -1 on error
 */
static int decode_spacepacket_header(uint8_t* encoded_buf,
                                     size_t encoded_buf_size,
                                     spacepacket_header_t* header_out) {
  // Sanity checks
  if (encoded_buf == NULL || header_out == NULL ||
      encoded_buf_size < SPACEPACKET_ENCODED_HEADER_SIZE) {
    return -1;
  }
  // Decode header
  header_out->version = (encoded_buf[0] >> 5) & 0x07;
  header_out->type = (encoded_buf[0] >> 4) & 0x01;
  header_out->secondary_header_flag = (encoded_buf[0] >> 3) & 0x01;
  header_out->apid = ((encoded_buf[0] & 0x07) << 8) | encoded_buf[1];
  header_out->sequence_flag = (encoded_buf[2] >> 6) & 0x01;
  header_out->packet_sequence_count =
      ((encoded_buf[2] & 0x3F) << 8) | encoded_buf[3];
  header_out->packet_length = (encoded_buf[4] << 8) | encoded_buf[5];
  return 0;
}

static spacepacket_header_t header;
static uint8_t* packet;
void decodeAndTransmit() {
  // decode - receive a packet - TODO: make this smarter

  packet = (uint8_t*)malloc(sizeof(uint8_t) * (4 + 6));
  uint8_t index = 0;

  // find sync bytes
  while (true) {
    uint8_t byte_in = Serial.read();

    if (byte_in == COMMAND_SYNC_BYTES[index]) {
      packet[index] = byte_in;
      index++;
    } else {
      index = 0;
    }

    if (index >= (sizeof(COMMAND_SYNC_BYTES) - 2)) {  // - 2 for \0 and counting
      break;
    }
  }

  // receive spacepacket header (6 bytes)
  for (int i = 0; i < SPACEPACKET_ENCODED_HEADER_SIZE; i++) {
    packet[index] = Serial.read();
    index++;
  }

  decode_spacepacket_header(packet + 4, SPACEPACKET_ENCODED_HEADER_SIZE,
                            &header);
  size_t full_len = header.packet_length + (sizeof(COMMAND_SYNC_BYTES) - 1) +
                    SPACEPACKET_ENCODED_HEADER_SIZE;

  // receive the rest of the packet
  packet = (uint8_t*)realloc(packet, full_len);

  while (index < full_len) {
    packet[index] = Serial.read();
    index++;
  }

  // transmit the entire buffer
  radio.startTransmit(packet, full_len);
}

void transmitCleanUp() {
  // clean up needed after every transmit
  radio.finishTransmit();
}

#define RADIO_SET_MODE 103
#define TOKEN_LENGTH 8U
typedef struct __attribute__((__packed__)) {
  uint8_t admin_token[TOKEN_LENGTH];
  uint8_t radio_mode;
} radio_set_mode_t;

void adaptRadio() {
  // use with decode
  if (header.apid == RADIO_SET_MODE) {
    uint8_t* buf = packet + ((sizeof(COMMAND_SYNC_BYTES) - 1) +
                             SPACEPACKET_ENCODED_HEADER_SIZE);
    radio_set_mode_t* set_mode_args = (radio_set_mode_t*)buf;

    if (set_mode_args->radio_mode == RADIO_FAST_MODE) {
      radio_begin_fast();
    } else {
      radio_begin_safe();
    }
  }

  // clean up from decode at the end
  free(packet);
}
