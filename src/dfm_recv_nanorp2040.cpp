/*
  FILE: DFM_RECV_NANORP2040.CPP
  VERSION: 0.0.1
  DATE: 25 January 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite
  DESCRIPTION:
*/
#include <dfm_recv_nanorp2040.h>

#ifdef BUILD_RECV_NANORP2040

#include <Arduino.h>
#include <WiFiNINA.h>

#define SERIALBAUD 115200

void setup_recv_nanorp2040() {

#ifdef DEBUG
    Serial.begin(SERIALBAUD);
    while (!Serial)
        ;
#endif
}
void loop_recv_nanorp2040() {}

#endif
