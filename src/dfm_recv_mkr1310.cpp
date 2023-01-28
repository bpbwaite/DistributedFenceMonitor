/*
  FILE: DFM_MKR1310.CPP
  VERSION: 0.0.4
  DATE: 28 January 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite
  DESCRIPTION: MKR1310 Central Node Code
*/
#include <dfm_mkr1310.h>
#if defined(ARDUINO_SAMD_MKRWAN1310) & defined(CENTRAL_NODE)

#include <Arduino.h>
#include <ArduinoECCX08.h>
#include <LoRa.h>
#include <SPI.h>

int counter = 0;

void setup_recv_mkr1310() {

#ifdef DEBUG
    Serial.begin(SERIALBAUD);
    while (!Serial)
        yield();
#endif

    LoRa.setGain(RECEIVER_GAINMODE);
    LoRa.setSpreadingFactor(SPREADFACTOR);
    LoRa.setSignalBandwidth(SIGNALBANDWIDTH);
    LoRa.setSyncWord(SYNCWORD);
    LoRa.setPreambleLength(PREAMBLELEN);

#if defined(USING_CRC)
    LoRa.enableCrc();
#else
    LoRa.disableCrc();
#endif
}
void loop_recv_mkr1310() {}

#endif
