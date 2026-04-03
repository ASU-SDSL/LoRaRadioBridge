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

  // default to receive mode
  mode_receive();
}

void mode_receive() {
  // wait for signal through safely 
  // ptt off
  digitalWrite(PTT_PIN, PTT_OFF); 
  // wait for ptt to ramp down
  delay(PTT_TIME_DOWN_MS);
  // switch rf switch to lna
  digitalWrite(SWITCH_PIN, SWITCH_ON);
  // wait for it to switch
  delay(SWITCHING_TIME_MS); 
}

void mode_transmit() {
  // switch rf switch to sink
  digitalWrite(SWITCH_PIN, SWITCH_OFF);
  // wait for it to switch
  delay(SWITCHING_TIME_MS); 
  // ptt on
  digitalWrite(PTT_PIN, PTT_ON); 
  // wait for ptt to ramp up?
  delay(PTT_TIME_UP_MS); 
}