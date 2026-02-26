#include <Arduino.h>
#include "Globals.h"

#define COMMAND_SYNC_BYTES "\x35\x2E\xF8\x53" 

static queue_t real_in_q; 
static queue_t real_out_q; 

queue_t* in_q = &real_in_q; 
queue_t* out_q = &real_out_q; 

void real_setup1(){
  queue_init(in_q, sizeof(msg_in_t), 20);
  queue_init(out_q, sizeof(msg_out_t), 20); 

  Serial.begin(115200); 
  while(!Serial); 

}

static msg_in_t msg; 

void real_loop1(){
  // read from serial 
  // make sure we have at least a sync (-1 for null term) 
  if(Serial.available() > sizeof(COMMAND_SYNC_BYTES) - 1){
    uint8_t pos = 0;  

    while(Serial.available() > 0 && pos < MAX_PACKET_SIZE){
      msg.data[pos++] = Serial.read();
    }

    queue_add_blocking(in_q, &msg); 
  }

  // write to serial 
  if(queue_is_empty(out_q) == false){
    msg_out_t msg; 

    queue_remove_blocking(out_q, &msg); 

    Serial.write(msg.data, msg.len); 
  }
}