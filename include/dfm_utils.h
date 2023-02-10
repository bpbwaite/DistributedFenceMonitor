/*
  FILE: DFM_UTILS.H
  VERSION: 0.0.1
  DATE: 10 February 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite
  DESCRIPTION: Functions for Distributed Fence Monitor
*/
#pragma once

#ifdef ARDUINO_SAMD_MKRWAN1310

#include <Arduino.h>
#include "dfm_mkr1310.h"

// Node Functions
uint8_t maxPayload(int = REGION_TAG, int = SPREADFACTOR, long = CHIRPBW);
double getTOA(int, int = SPREADFACTOR, long = CHIRPBW, int = PREAMBLELEN, float = CODERATE, bool = USING_CRC);

// Receiver Functions
void epchtostr(char *, uint32_t);
void mndtostr(char *, const MonitoringNodeData);

// Shared Functions
void indicateOn();
void indicateOff();

#endif
