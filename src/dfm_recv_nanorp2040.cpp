/*
  FILE: DFM_RECV_NANORP2040.CPP
  VERSION: 0.0.2
  DATE: 25 January 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite
  DESCRIPTION: NanoRP2040 Central Node Code
*/
#include <dfm_nanorp2040.h>
#if defined(ARDUINO_NANO_RP2040_CONNECT) && defined(CENTRAL_NODE)

#include <Arduino.h>
#include <WiFiNINA.h>

void setup_recv_nanorp2040() {

#ifdef DEBUG
    Serial.begin(SERIALBAUD);
    while (!Serial)
        ;
#endif
}
void loop_recv_nanorp2040() {}

#endif
