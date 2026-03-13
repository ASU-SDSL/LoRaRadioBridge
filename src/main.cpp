#include <Arduino.h>
#include <RadioLib.h>

#include "Globals.h"
#include "HardwareConfig.h"
#include "PicoHal.h"
#include "modes.h"

#define RADIO_FREQ 437.400
#define RADIO_BW 62.5
#define RADIO_SF 10
#define RADIO_CR 5
#define RADIO_SYNC_WORD 18
#define RADIO_PREAMBLE_LEN 8

#define RADIO_RFM_GAIN 0
#define RADIO_SX_TXCO_VOLT 0.0
#define RADIO_SX_USE_REG_LDO false
#define RADIO_SX_POWER 22   // to get 30dBm
#define RADIO_RFM_POWER 17  // to get 30dBm

#define RECEIVE_TIMEOUT_MS 5000
#define TRANSMIT_TIMEOUT_MS 5000

PicoHal* txHal = new PicoHal(spi0, PICO_DEFAULT_SPI_TX_PIN,
                             PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_SCK_PIN);
PicoHal* rxHal = new PicoHal(spi1, SPI1_MOSI_PIN, SPI1_MISO_PIN, SPI1_SCK_PIN);

SX1268 radioTX = new Module(txHal, RADIO_SX_CS_PIN, RADIO_SX_DIO1_PIN,
                            RADIO_SX_NRESET_PIN, RADIO_SX_BUSY_PIN);
RFM98 radioRX = new Module(rxHal, RADIO_RFM_CS_PIN, RADIO_RFM_DIO0_PIN,
                           RADIO_RFM_RESET_PIN, RADIO_RFM_DIO1_PIN);

// isrs
volatile bool operation_done_RFM = false;
volatile bool cad_detected_RFM = false;
volatile bool general_flag_SX = false;

void radio_operation_done_RFM() { operation_done_RFM = true; }

void radio_cad_detected_RFM() { cad_detected_RFM = true; }

void radio_general_flag_SX() { general_flag_SX = true; }
// end isrs

uint32_t op_start = 0;

void setup() {
  // the init state
  while (!Serial) delay(100);
  delay(500);

  pinMode(STATUS_PIN, OUTPUT);

  mode_init();

  // radio setup
  // spi setup
  SPI.begin();

  SPI1.setMISO(SPI1_MISO_PIN);
  SPI1.setMOSI(SPI1_MOSI_PIN);
  SPI1.setSCK(SPI1_SCK_PIN);
  SPI1.begin();

  // module begin
  // tx
  int res = radioTX.begin(RADIO_FREQ, RADIO_BW, RADIO_SF, RADIO_CR,
                          RADIO_SYNC_WORD, RADIO_SX_POWER, RADIO_PREAMBLE_LEN,
                          RADIO_SX_TXCO_VOLT, RADIO_SX_USE_REG_LDO);
  if (res != RADIOLIB_ERR_NONE) {
    Serial.printf("Error on TX (SX): %d\n", res);
  }
  radioTX.standby();

  // rx
  res = radioRX.begin(RADIO_FREQ, RADIO_BW, RADIO_SF, RADIO_CR, RADIO_SYNC_WORD,
                      RADIO_RFM_POWER, RADIO_PREAMBLE_LEN, RADIO_RFM_GAIN);
  if (res != RADIOLIB_ERR_NONE) {
    Serial.printf("Error on RX (RFM): %d\n", res);
  }

  // setup interrupts
  radioRX.setDio0Action(radio_operation_done_RFM, GPIO_IRQ_EDGE_RISE);
  radioRX.setDio1Action(radio_cad_detected_RFM, GPIO_IRQ_EDGE_RISE);

  // probably not needed - transmissions should block anyways
  radioTX.setDio1Action(radio_general_flag_SX);

  // start CAD
  res = radioRX.startChannelScan();
  op_start = millis();

  if (res != RADIOLIB_ERR_NONE) {
    Serial.printf("Error on CAD start %d\n", res);
  }

}

enum stage_t { CAD, TRANSMITTING, RECEIVING };

// only calls this, not startTransmit directly
int16_t safe_startTransmit(const uint8_t* data, uint8_t len) {
  // handles switching delay
  mode_transmit();

  return radioTX.startTransmit(data, len);
}

stage_t current_state = stage_t::CAD;

void loop() {
  // vars
  msg_in_t msg_in;
  msg_out_t msg_out;

  switch (current_state) {
    case CAD:
      if (cad_detected_RFM) {
        cad_detected_RFM = false;  // clear flag
        operation_done_RFM = false;

        // cad detected
        current_state = stage_t::RECEIVING;

        // start receive
        radioRX.startReceive();
        op_start = millis();
      } else if (operation_done_RFM) { // CAD is done, didn't see any activity
        operation_done_RFM = false;  // clear flag

        if (queue_is_empty(in_q) == false) {
          // have packet to send
          // receive packet from core 1
          queue_remove_blocking(in_q, &msg_in);

          safe_startTransmit(msg_in.data, msg_in.len);
          op_start = millis();
          current_state = stage_t::TRANSMITTING;
        } else {
          // cad operation done, repeat
          radioRX.startChannelScan();
          current_state = stage_t::CAD;
        }
      }

      break;
    case TRANSMITTING:
      if (millis() - op_start > TRANSMIT_TIMEOUT_MS) {
        // timeout
        radioTX.finishTransmit();

        // return to CAD
        mode_receive();
        radioRX.startChannelScan();
        current_state = stage_t::CAD;
      } else if (general_flag_SX) {
        general_flag_SX = false;  // clear flag

        radioTX.finishTransmit();

        // return to CAD
        mode_receive();
        radioRX.startChannelScan();
        current_state = stage_t::CAD;
      }

      break;
    case RECEIVING:
      if (millis() - op_start > RECEIVE_TIMEOUT_MS) {
        // timeout
        radioRX.finishReceive();

        // return to CAD
        radioRX.startChannelScan();
        current_state = stage_t::CAD;
      } else if (operation_done_RFM) {
        operation_done_RFM = false;  // clear flag

        // received a packet
        msg_out.len = radioRX.getPacketLength();
        radioRX.readData(msg_out.data, msg_out.len);

        radioRX.finishReceive();

        // send to core1
        queue_add_blocking(out_q, &msg_out);

        // return to CAD
        radioRX.startChannelScan();
        current_state = stage_t::CAD;
      }

      break;
    default:
      Serial.println("Error - bad state");
  }
}

// Core 1 stuff --------------------------------------------
void real_setup1();
void real_loop1();

void setup1() { real_setup1(); }

void loop1() { real_loop1(); }