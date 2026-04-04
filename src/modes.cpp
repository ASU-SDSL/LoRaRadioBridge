#include "modes.h"

#include "HardwareConfig.h"

// ptt ramp down + relay switch + rf switch
// needs to be less than ~50ms to see acks 
// relay switch + rf switch worse case (10ms for each)
#define SWITCHING_TIME_MS 20  

// ptt ramp times 
#define PTT_TIME_UP_MS 100  
#define PTT_TIME_DOWN_MS 25

void mode_init() {
  pinMode(SWITCH_PIN, OUTPUT);
  pinMode(PTT_PIN, OUTPUT); 
  pinMode(SWITCH_INDICATOR_PIN, OUTPUT);
  pinMode(PTT_INDICATOR_PIN, OUTPUT);

  // default to receive mode
  mode_receive();
}

static void set_ptt(int status) {
  digitalWrite(PTT_PIN, status);
  digitalWrite(PTT_INDICATOR_PIN, status);
}

static void set_switch(int status) {
  digitalWrite(SWITCH_PIN, status);
  digitalWrite(SWITCH_INDICATOR_PIN, status); 
}

void mode_receive() {
  // wait for signal through safely 
  // ptt off
  set_ptt(PTT_OFF); 
  // wait for ptt to ramp down
  delay(PTT_TIME_DOWN_MS);
  // switch rf switch to lna
  set_switch(SWITCH_ON);
  // wait for it to switch
  delay(SWITCHING_TIME_MS); 
}

void mode_transmit() {
  // switch rf switch to sink
  set_switch(SWITCH_OFF);
  // wait for it to switch
  delay(SWITCHING_TIME_MS); 
  // ptt on
  set_ptt(PTT_ON); 
  // wait for ptt to ramp up?
  delay(PTT_TIME_UP_MS); 
}