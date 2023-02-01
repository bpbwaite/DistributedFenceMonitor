/*
  FILE: DFM_MKR1310.H
  VERSION: 0.0.6
  DATE: 28 January 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite, Jack Ramsay
  DESCRIPTION: MKR1310 Boards Configuration Defines
*/
#pragma once

#ifdef ARDUINO_SAMD_MKRWAN1310

#include <Arduino.h>
#include <LoRa.h>

#define PIN_LORAMODE  2
#define PIN_DISCRETE  0
#define PIN_STATUSLED 6
#define PIN_ERRORLED  3
#define PIN_BATADC    A1

#define DEBUG      true
#define SERIALBAUD 115200
// the following settings must match on the sencer and receiver
#define USING_CRC true
const double LORA_AMERICA    = 915E6;
const double LORA_AFRICA     = 868E6;
const double LORA_EUROPE     = 433E6;
const double SIGNALBANDWIDTH = 125E3; // default 125E3. among other values, can also be 250E3 or 500E3.

#define SPREADFACTOR 10
// ranges from 6-12, default 7, sender/receiver must match.
// higher not necessarily = further, but usually better strength (?)
// The duration of a symbol is 2^SF / BW (SF: Spreading Factor, BW: Bandwidth)

#define SYNCWORD          0x12 // default is 0x12, 0x34 is reserved for public communications, probably a byte
#define PREAMBLELEN       8    // 6-65535, default 8
#define RECEIVER_GAINMODE 0    // Range 1-6. 0 is automatic. Applies to receiver only

typedef struct {
    uint16_t ID;
    uint32_t packetnum;
    byte status;
    uint16_t connectedNodes;
    byte bat;
    byte freq;
    byte SF;
    byte SyncWord;
    uint32_t upTime;
    uint32_t timeInTransit;
    byte temp;
    uint32_t epoch;
} MonitoringNodeData;

void setup_mkr1310();
void loop_mkr1310();
#endif
