// based on channel activity detection interrupt example
// include the library
#include <RadioLib.h>

// SX1278 has the following connections:
// NSS pin:   10
// DIO0 pin:  2
// RESET pin: 9
// DIO1 pin:  3
RFM98 radio = new Module(10, 2, 9, 3);

// flag to indicate that a preamble was not detected
volatile bool timeoutFlag = false;

// flag to indicate that a preamble was detected
volatile bool detectedFlag = false;

bool receiving = false;

void setFlagTimeout(void) {
  // we timed out, set the flag
  timeoutFlag = true;
}

void setFlagDetected(void) {
  // we got a preamble, set the flag
  detectedFlag = true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE) {
    //    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true)
      ;
  }

  // set the function that will be called
  // when LoRa preamble is not detected within CAD timeout period
  radio.setDio0Action(setFlagTimeout, RISING);

  // set the function that will be called
  // when LoRa preamble is detected
  radio.setDio1Action(setFlagDetected, RISING);

  state = radio.startChannelScan();
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("Error: ");
    Serial.println(state);
  }
//  Serial.println("Setup /done");
}

void loop() {
  // if we're not receiving, and there are bytes to send 
  if(!receiving && Serial.available() > 0){
    // large buffer - radio's buffer
    uint8_t buf[256];
    size_t len = 0; 
    // while there are still bytes to send
    while(Serial.available() > 0 && len < 256){
      // read them into the buffer
      buf[len++] = Serial.read(); 
    }
    // transmit the entire buffer
    radio.transmit(buf, len); 
  }

  if (detectedFlag || timeoutFlag) {
    if (receiving) {
      // reset flags
      timeoutFlag = false;
      detectedFlag = false;

      size_t len = radio.getPacketLength();
      uint8_t buf[len];
      int state = radio.readData(buf, len);

      if (state == RADIOLIB_ERR_NONE) {
        Serial.write(buf, len);
      } else {
//        Serial.print("Error on receive: ");
//        Serial.println(state);
      }

      receiving = false;
    }

    // check if we got a preamble
    if (detectedFlag) {
      // LoRa preamble was detected
      //      Serial.println(F("[SX1278] Preamble detected!"));
      radio.startReceive(0, RADIOLIB_SX127X_RXSINGLE);

      receiving = true;
    }

    if (!receiving) {
      // start scanning current channel
      int state = radio.startChannelScan();
      if (state == RADIOLIB_ERR_NONE) {
        //      Serial.println(F("success!"));
      } else {
//        Serial.print(F("failed, code "));
//        Serial.println(state);
      }
    }

    // reset flags
    timeoutFlag = false;
    detectedFlag = false;
  }
}
