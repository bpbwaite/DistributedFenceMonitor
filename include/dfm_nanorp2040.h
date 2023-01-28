/*
  FILE: DFM_NANORP2040.H
  VERSION: 0.0.5
  DATE: 27 January 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite
  DESCRIPTION:
*/
#pragma once

#ifdef ARDUINO_NANO_RP2040_CONNECT

#define SERIALBAUD 115200

void setup_nanorp2040();
void loop_nanorp2040();
#endif
