/*
  FILE: DFM_UTILS.H
  VERSION: 0.0.5
  DATE: 22 February 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite
  DESCRIPTION: Functions for Distributed Fence Monitor
*/
#pragma once

#ifdef ARDUINO_SAMD_MKRWAN1310

#include "dfm_mkr1310.h"
#include <Arduino.h>

// Structures
typedef struct {
    int32_t rssi;
    float snr;
    uint32_t bw;
    uint8_t sf;

} ReceiverExtras;

// Node Functions
uint8_t maxPayload(int = REGION_TAG, int = SPREADFACTOR, long = CHIRPBW);
double getTOA(int, int = SPREADFACTOR, long = CHIRPBW, int = PREAMBLELEN, float = CODERATE, bool = USING_CRC);

// Receiver Functions
void epchtostr(char *, uint32_t);                   // deprecated
void mndtostr(Serial_ &, const MonitoringNodeData); // deprecated
void mndtomatlab(Serial_ &, const MonitoringNodeData, const ReceiverExtras);

// Shared Functions
void indicateOn();
void indicateOff();

#endif
