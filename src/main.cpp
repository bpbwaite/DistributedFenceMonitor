/*
  FILE: MAIN.CPP
  VERSION: 1.0.0
  DATE: 3 March 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite
  DESCRIPTION:
*/

#include <Arduino.h>

#include "dfm_mkr1310.h"
#include "dfm_nanorp2040.h"
#include "dfm_recv_mkr1310.h"
#include "dfm_recv_nanorp2040.h"

#ifndef PIO_UNIT_TESTING

void setup() {
    // put your setup code here, to run once:
#if defined(ARDUINO_SAMD_MKRWAN1310)
#if defined(CENTRAL_NODE)
    setup_recv_mkr1310();
#else
    setup_mkr1310();
#endif
#elif defined(ARDUINO_NANO_RP2040_CONNECT)
#if defined(CENTRAL_NODE)
    setup_recv_nanorp2040();
#else
    setup_nanorp2040();
#endif
#endif
}

void loop() {
    // put your main code here, to run repeatedly:
#if defined(ARDUINO_SAMD_MKRWAN1310)
#if defined(CENTRAL_NODE)
    loop_recv_mkr1310();
#else
    loop_mkr1310();
#endif
#elif defined(ARDUINO_NANO_RP2040_CONNECT)
#if defined(CENTRAL_NODE)
    loop_recv_nanorp2040();
#else
    loop_nanorp2040();
#endif
#endif
}

#endif
