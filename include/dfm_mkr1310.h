/*
  FILE: DFM_MKR1310.H
  VERSION: 0.0.5
  DATE: 27 January 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite
  DESCRIPTION:
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

#define SERIALBAUD   115200
#define LORA_AMERICA 915E6
#define LORA_AFRICA  868E6
#define LORA_EUROPE  433E6

typedef struct {
    uint16_t ID;
    uint32_t packetnum;
    byte status;
    uint16_t connectedNodes;
    byte bat;
    byte freq;
    float SNR;
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
