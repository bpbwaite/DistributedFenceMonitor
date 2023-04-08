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

#include <Arduino.h>
#include <SparkFun_ADXL345.h>
#include "dfm_mkr1310.h"

// Structures

typedef struct {
    // Version of MonitoringNodeData that stores
    // Information in as few bits as possible
    uint32_t packetnum;  // 32 bits for packet number that increments with each transmission
    uint32_t upTime;     // 32 bits for uptime in milliseconds
    uint32_t epoch;      // 32 bits for global time since 1970
    uint32_t all_states; // see compact status byte protocol
    uint16_t ID;         // 16 bits for ID of each node indicating which node it is
    uint16_t reserved;   // 16 bits for future use
    // syncword is already known by receiver by nature of having received the packet
    // freq is already known by receiver by nature of having received the packet
    // timeOnAir can be computed by the receiver
} MND_Compact;

typedef struct {

    // what gets reported from the central node to central server
    // everything shall be 32 bits. only the sign matters; size doesn't, alignment does.
    // physical stats
    uint32_t ID;
    uint32_t packetnum;
    uint32_t bat;
    uint32_t temperature;

    // indicators
    uint32_t severity;
    uint32_t connectedNodes;
    uint32_t hasAccel;
    uint32_t wantsRTC;
    uint32_t upTime;
    uint32_t epoch;
    uint32_t minSinceCal;

    // lora stats
    uint32_t sw;
    uint32_t bw;
    int32_t freq;
    int32_t rssi;

    // not yet implemented
    uint32_t hops;

} MND_Report;

// Deprecated:
// typedef struct {
//     uint32_t packetnum;
//     uint16_t ID;
//     uint16_t connectedNodes;
//     uint16_t status;
//     uint16_t SyncWord;
//     uint32_t bat;
//     int32_t freq;
//     uint32_t upTime;
//     uint32_t timeOnAir;
//     float temperature;
//     uint32_t epoch;
//     // note: alignment dictates 4 byte blocks
// } MonitoringNodeData;

typedef struct {
    int32_t rssi;
    float snr;
    uint32_t bw;
    uint8_t sf;

} ReceiverExtras;

typedef struct {
    int32_t panelNum;
    int32_t rssi;
    int32_t txpow;
    float snr;

} TestPing;

typedef struct {
    int x;
    int y;
    int z;

} AccelData;

// Node Functions

void showtime();

uint8_t maxPayload(int = REGION_TAG_US, int = SPREADFACTOR, long = CHIRPBW);
double getTOA(int, int = SPREADFACTOR, long = CHIRPBW, int = PREAMBLELEN, float = CODERATE, bool = USING_CRC);

void fullResetADXL(ADXL345 *);
void adxlMode(ADXL345 *, uint8_t);

double bwCodeToFs(byte);
int getDCOffset(ADXL345 *, double);

bool inactivityInDataEnd(double *, double, ADXL345 *adxl);

void setSeverity(MND_Compact &, int);
void setTSLC(MND_Compact &, int);
void setNeedRTC(MND_Compact &, bool);
void setTemperature(MND_Compact &, int);
void setIMUBit(MND_Compact &, bool);
void setBatt(MND_Compact &, int);
void setConnections(MND_Compact &, int);

// Receiver Functions
void epchtostr(char *, uint32_t); // deprecated
// void mndtostr(Serial_ &, const MonitoringNodeData); // deprecated
// void mndtomatlab(Serial_ &, const MonitoringNodeData, const ReceiverExtras);

// Shared Functions
void indicateOn();
void indicateOff();
void errorOn();
void errorOff();
void ERROR_OUT(uint8_t = 0b10001000);

int getSeverity(const MND_Compact &);
int getTSLC(const MND_Compact &);
bool getNeedRTC(const MND_Compact &);
int getTemperature(const MND_Compact &);
bool getIMUBit(const MND_Compact &);
int getBatt(const MND_Compact &);
int getConnections(const MND_Compact &);

#endif
