#include "modes.h"
#include "HardwareConfig.h"

void mode_init(){
  pinMode(LNA_PIN, OUTPUT); 
  pinMode(SWITCH_PIN, OUTPUT); 

  digitalWrite(LNA_PIN, LNA_OFF); 
  digitalWrite(SWITCH_PIN, SWITCH_OFF); 
}

void mode_receive(){
  digitalWrite(LNA_PIN, LNA_ON);
  digitalWrite(SWITCH_PIN, SWITCH_ON); 
}

void mode_transmit(){
  digitalWrite(LNA_PIN, LNA_OFF); 
  digitalWrite(SWITCH_PIN, SWITCH_OFF); 
}