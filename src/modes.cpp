#include "modes.h"

#include "HardwareConfig.h"

#define SWITCHING_TIME_MS 50
#define PTT_TIME_UP_MS 50
#define PTT_TIME_DOWN_MS 20

void mode_init() {
  pinMode(SWITCH_PIN, OUTPUT);
  pinMode(PTT_PIN, OUTPUT); 

  digitalWrite(SWITCH_PIN, SWITCH_OFF);
}

void mode_receive() {
  // wait for signal through safely 
  delay(PTT_TIME_DOWN_MS); 
  // ptt off
  digitalWrite(PTT_PIN, PTT_OFF); 
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
  // ptt on
  digitalWrite(PTT_PIN, PTT_ON); 
  // wait for ptt to ramp up?
  delay(PTT_TIME_UP_MS); 
}