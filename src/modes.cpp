#include "modes.h"

#include "HardwareConfig.h"

#define SWITCHING_TIME_MS 50

void mode_init() {
  pinMode(SWITCH_PIN, OUTPUT);

  digitalWrite(SWITCH_PIN, SWITCH_OFF);
}

void mode_receive() {
  // switch rf switch to lna
  digitalWrite(SWITCH_PIN, SWITCH_ON);
  // wait for it to switch
  delay(SWITCHING_TIME_MS);  // TODO: find exact time
}

void mode_transmit() {
  // switch rf switch to sink
  digitalWrite(SWITCH_PIN, SWITCH_OFF);
  // wait for it to switch
  delay(SWITCHING_TIME_MS);  // TODO: find exact time
}