/*
  FILE: DFM_MKR1310.H
  VERSION: 0.2.0
  DATE: 10 February 2023
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
#define PIN_BATADC    A1 // not to be confused with ADC_BATTERY

#define DEBUG         true
#define SERIALBAUD    115200
#define SERIALTIMEOUT 10000
// the following settings must match on the sencer and receiver
#define USING_CRC true
#define GMTOFFSET -25200

#define NUMCHANNELS_US 64
#define NUMCHANNELS_EU 9

#define CHIRPBW 125000UL // default 125E3. express as UL
// among other values, can also be 250E3 or 500E3.
// cannot be 500E3 in the EU

#define SPREADFACTOR 7
// ranges from 7-12, default 7, sender/receiver must match.
// The duration of a symbol is 2^SF / BW (SF: Spreading Factor, BW: Bandwidth)
// a symbol is a byte packed as a two-nibble pair

#define REGION_TAG  915    // 915 for US, 868 for EU
#define SYNCWORD    0x0012 // default is 0x12, 0x34 is reserved for public communications. 2 bytes
#define PREAMBLELEN 8      // 6-65535, default 8. symbols of 8 bits each
#define CRCLEN      2      // bytes
#define CODERATE    5      // represents 4/5

#define RECEIVER_GAINMODE 0 // Range 1-6. 0 is automatic. Applies to receiver only

// note: alignment dictates 4 byte blocks
typedef struct {
    uint32_t packetnum;
    uint16_t ID;
    uint16_t connectedNodes;
    uint16_t status;
    uint16_t SyncWord;
    uint32_t bat;
    int32_t freq;
    uint32_t upTime;
    uint32_t timeOnAir;
    float temperature;
    uint32_t epoch;

} MonitoringNodeData;

const long LoRaChannelsUS[] = {
    902300000, 902500000, 902700000, 902900000, 903100000, 903300000, 903500000, 903700000, 903900000, 904100000,
    904300000, 904500000, 904700000, 904900000, 905100000, 905300000, 905500000, 905700000, 905900000, 906100000,
    906300000, 906500000, 906700000, 906900000, 907100000, 907300000, 907500000, 907700000, 907900000, 908100000,
    908300000, 908500000, 908700000, 908900000, 909100000, 909300000, 909500000, 909700000, 909900000, 910100000,
    910300000, 910500000, 910700000, 910900000, 911100000, 911300000, 911500000, 911700000, 911900000, 912100000,
    912300000, 912500000, 912700000, 912900000, 913100000, 913300000, 913500000, 913700000, 913900000, 914100000,
    914300000, 914500000, 914700000, 914900000,
};
const long LoRaChannelsEU[] = {
    868100000,
    868300000,
    868500000,
    867100000,
    867300000,
    867500000,
    867700000,
    867900000,
    868800000,
};

void setup_mkr1310();
void loop_mkr1310();
#endif
