/*
  FILE: MAIN.CPP
  VERSION: 1.5.0
  DATE: 22 March 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite
  DESCRIPTION:
*/

#include <Arduino.h>

#include "dfm_mkr1310.h"
#include "dfm_recv_mkr1310.h"

#ifndef PIO_UNIT_TESTING

void setup() {
    // put your setup code here, to run once:
#if defined(ARDUINO_SAMD_MKRWAN1310)
#if defined(CENTRAL_NODE)
    setup_recv_mkr1310();
#else
    setup_mkr1310();
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
#endif
}

#endif
