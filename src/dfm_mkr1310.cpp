/*
  FILE: DFM_MKR1310.CPP
  VERSION: 0.0.2
  DATE: 25 January 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite, Jack Ramsay
  DESCRIPTION: Minimal test code for MKR1310
*/
#include <dfm_mkr1310.h>
#if defined(ARDUINO_SAMD_MKRWAN1310) & !defined(CENTRAL_HUB)

#include <Arduino.h>
#include <ArduinoECCX08.h>
#include <ArduinoLowPower.h>
#include <LoRa.h>
#include <SPI.h>



int counter = 0;

void setup_mkr1310() {

#ifdef DEBUG
    Serial.begin(SERIALBAUD);
    while (!Serial)
        ;
#endif

    if (!LoRa.begin(LORA_AMERICA)) {
        Serial.println("LoRa Module Failure");
    }
    else {
        Serial.println("LoRa Module Online");
    }

    LoRa.setGain(5);             // range 0-6
    LoRa.setSpreadingFactor(12); // ranges from 6-12,default 7, sender/receiver must match
    LoRa.packetSnr();
}
void loop_mkr1310() {}

#endif
