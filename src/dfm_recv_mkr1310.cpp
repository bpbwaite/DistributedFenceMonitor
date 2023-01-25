/*
  FILE: DFM_MKR1310.CPP
  VERSION: 0.0.1
  DATE: 25 January 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite
  DESCRIPTION:
*/
#include <dfm_recv_mkr1310.h>
#ifdef BUILD_RECV_MKR1310

#include <Arduino.h>
#include <ArduinoECCX08.h>
#include <LoRa.h>
#include <SPI.h>

#define SERIALBAUD 115200

#define LORA_AMERICA 915E6
#define LORA_AFRICA  868E6
#define LORA_EUROPE  433E6

int counter = 0;

void setup_recv_mkr1310() {

#ifdef DEBUG
    Serial.begin(SERIALBAUD);
    while (!Serial)
        ;
#endif
}
void loop_recv_mkr1310() {}

#endif
