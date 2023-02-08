/*
  FILE: DFM_MKR1310.H
  VERSION: 0.1.0
  DATE: 7 February 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite, Jack Ramsay
  DESCRIPTION: MKR1310 Boards Configuration Defines
*/
#pragma once

#ifdef ARDUINO_SAMD_MKRWAN1310

#include <Arduino.h>
#include <LoRa.h>
#include <WString.h>

#define PIN_LORAMODE  2
#define PIN_DISCRETE  0
#define PIN_STATUSLED 6
#define PIN_ERRORLED  3
#define PIN_BATADC    A1

#define DEBUG         true
#define SERIALBAUD    115200
#define SERIALTIMEOUT 10000
// the following settings must match on the sencer and receiver
#define USING_CRC true
#define GMTOFFSET -25200

#define LORA_AMERICA 915E6
#define LORA_AFRICA  868E6
#define LORA_EUROPE  433E6
#define MEGAHERTZ    1E6

#define SIGNALBANDWIDTH 125E3 // default 125E3. among other values, can also be 250E3 or 500E3.
// cannot be 500E3 in the European Union! Illegal!

#define SPREADFACTOR 7
// ranges from 7-12, default 7, sender/receiver must match.
// The duration of a symbol is 2^SF / BW (SF: Spreading Factor, BW: Bandwidth)
// a symbol is a byte packed as a two-nibble pair

#define SYNCWORD          0x12 // default is 0x12, 0x34 is reserved for public communications, probably a byte
#define PREAMBLELEN       8    // 6-65535, default 8. symbols of 8 bits each
#define CRCLEN            2    // bytes
#define RECEIVER_GAINMODE 0    // Range 1-6. 0 is automatic. Applies to receiver only

typedef struct {
    uint16_t ID;
    uint32_t packetnum;
    byte status;
    uint16_t connectedNodes;
    uint32_t bat;
    byte freq;
    byte channel;
    byte SyncWord;
    uint32_t upTime;
    uint32_t timeOnAir;
    float temperature;
    float accelX;
    float accelY;
    float accelZ;
    uint32_t epoch;

} MonitoringNodeData;

void setup_mkr1310();
void loop_mkr1310();
#endif
