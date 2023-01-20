/*
  FILE: DFM_MKR1310.CPP
  VERSION: 0.0.2
  DATE: 20 January 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite
  DESCRIPTION: Minimal test code for MKR1310
*/
#include <dfm_mkr1310.h>

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

void setup_mkr1310() {
    pinMode(LED_BUILTIN, OUTPUT);
}
void loop_mkr1310() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
}
