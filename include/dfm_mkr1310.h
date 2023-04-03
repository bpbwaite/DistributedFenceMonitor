/*
  FILE: DFM_MKR1310.H
  VERSION: 0.3.0
  DATE: 18 March 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite, Jack Ramsay
  DESCRIPTION: MKR1310 Boards Configuration Defines
*/
#pragma once

#ifdef ARDUINO_SAMD_MKRWAN1310

#include <Arduino.h>

// GENERAL SYSTEM CONFIGURATION
#define DEBUG         true
#define SERIALBAUD    115200
#define SERIALTIMEOUT 2500

// TIMING CONFIGURATION
#define SLEEP_TIME_MS       2000
#define GMTOFFSET           -25200
#define ADXL_SAMPLE_LENGTH  1000 // The number of accelerometer readings to hold
#define ADXL_DC_CAPTURE     500  // Samples to take when running DC calibration
#define ADXL_SAMPLE_TIMEOUT 500  // ms

// CALIBRATOR SETTINGS
#define CALIBRATION_TIME_SLICE    0.75     // seconds
#define ADXL_CALIBRATION_INTERVAL 900000UL // ms, 900000 ~= 15 minutes

// BATTERY POWER CONFIGURATION
#define VBAT_HUNDRED 4.10
#define VBAT_ZERO    3.30
#define ADC_BITS     10 // see about setting to 12
#define ADC_VREF     3.30
#define R_bot        680000.0
#define R_top        330000.0

// VALUES DETERMINED BY PCB LAYOUT
#define PIN_LORAMODE  2
#define PIN_DISCRETE  0
#define PIN_STATUSLED 6 // also shows the status of INT2
#define PIN_ERRORLED  3
#define PIN_BATADC    A1 // not to be confused with ADC_BATTERY
#define PIN_SW1       A5
#define PIN_ADXLCS    1  // legacy chipselect pin for the accelerometer
#define PIN_ADXLCS1   A3 // chipselect pin for the primary accelerometer
#define PIN_ADXLCS2   A4 // chipselect pin for the secondary accelerometer
#define PIN_INTERRUPT 7

#define ADXL_VOLTAGE 303 // 303 for 3.3, 205 for 2.5

// INERTIAL MEASUREMENT CONSTANTS
#define ADXL_FALLING 1
#define ADXL_RISING  0
// linspace(0, (2*9.81)^2, 16)
const double thresholdZ[15] = {
    25.66,
    51.32,
    76.98,
    102.65,
    128.31,
    153.97,
    179.64,
    205.30,
    230.96,
    256.62,
    282.29,
    307.95,
    333.61,
    359.28,
    384.94,
};
const double thresholdZ_logarithmic[15] = {
    1.4507,
    2.1046,
    3.0532,
    4.4294,
    6.4259,
    9.3223,
    13.5242,
    19.6200,
    28.4634,
    41.2927,
    59.9047,
    86.9057,
    126.0770,
    182.9040,
    265.3448,
};

// INERTIAL MEASUREMENT SETTINGS
#define ADXL_SENSITIVITY 2 // 2, 4, 8, 16 (g)
#define ADXL_FULLRESBIT  1
#define ADXL_ACT_THRESH  0x0006 // 62.5mg per increment
#define ADXL_TIME_REST   2
#define ADXL_LSB_PER_G_Z 256.0
#define GRAVITY          9.81

// LORA MODULE SETTINGS & CONFIG
#define LORA_POWER 4 // 2-17 or 20
// the following settings must match on the sender and receiver
#define USING_CRC false
#define CHIRPBW   125000UL
// default 125E3. express as UL
// among other values, can also be 250E3 or 500E3.
// cannot be 500E3 in the EU
#define SPREADFACTOR 7
// ranges from 7-12, default 7, sender/receiver must match.
// The duration of a symbol is 2^SF / BW (SF: Spreading Factor, BW: Bandwidth)
#define REGION_TAG_US     915    // 915 for US, 868 for EU
#define REGION_TAG_EU     868    // 915 for US, 868 for EU
#define SYNCWORD          0x0012 // default is 0x12, 0x34 is reserved for public communications. 2 bytes
#define PREAMBLELEN       8      // 6-65535, default 8. symbols of 8 bits each
#define CRCLEN            2      // bytes of CRC
#define CODERATE          5      // represents 4/5
#define RECEIVER_GAINMODE 0      // Range 1-6. 0 is automatic. Applies to receiver only

// LORA MODULE CONSTANTS
#define NUMCHANNELS_US 64
#define NUMCHANNELS_EU 9

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
