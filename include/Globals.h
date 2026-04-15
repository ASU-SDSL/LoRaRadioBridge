#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <RadioLib.h>

#include "pico/util/queue.h"

#define LOGGER_ENABLE 0
#define log(fmt, ...) \
  if (LOGGER_ENABLE)  \
    Serial.printf("[Core %d]" fmt, get_core_num(), ##__VA_ARGS__);

#define MAX_PACKET_SIZE 256

enum PacketType { COMMAND, DATA };

struct msg_in_t {
  uint8_t type;
  uint8_t len;
  uint8_t data[MAX_PACKET_SIZE];
};

struct msg_out_t {
  uint8_t len;
  uint8_t data[MAX_PACKET_SIZE];
};

extern queue_t* in_q;
extern queue_t* out_q;

#endif