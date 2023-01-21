/*
  FILE: DFM_MKR1310.CPP
  VERSION: 0.0.2
  DATE: 20 January 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite
  DESCRIPTION: Minimal test code for MKR1310
*/
#include <dfm_mkr1310.h>

#ifdef BUILD_MKR1310

#include <Arduino.h>
#include <ArduinoECCX08.h>
#include <LoRa.h>
#include <SPI.h>

#define SERIALBAUD 115200

#define LORA_AMERICAN 915E6
#define LORA_AFRICAN  868E6
#define LORA_EUROPEAN 433E6

int counter = 0;

void setup_mkr1310() {
#ifdef DEBUG
    Serial.begin(SERIALBAUD);
    while (!Serial)
        ;
#endif

    if (!LoRa.begin(LORA_AMERICAN)) {
        Serial.println("LoRa Module Failure");
    }
    else {
        Serial.println("LoRa Module Online");
    }

    LoRa.setGain(5);            // range 0-6
    LoRa.setSpreadingFactor(7); // ranges from 6-12,default 7, sender/receiver must match
}
void loop_mkr1310() {
    LoRa.beginPacket();
    LoRa.print("hello ");
    LoRa.print(counter);
    LoRa.endPacket();

    counter++;

    delay(5000);
}

#endif
