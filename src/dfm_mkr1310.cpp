/*
  FILE: DFM_MKR1310.CPP
  VERSION: 0.1.0
  DATE: 7 February 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite, Jack Ramsay
  DESCRIPTION: Preliminary test code for MKR1310
*/
#include "dfm_mkr1310.h"
#if defined(ARDUINO_SAMD_MKRWAN1310) && !defined(CENTRAL_NODE)

#include <Adafruit_ADXL345_U.h>
#include <Arduino.h>
#include <ArduinoECCX08.h>
#include <ArduinoLowPower.h>
#include <LoRa.h>
#include <RTCZero.h>
#include <SPI.h>
#include <time.h>

// only defines unique to nodes that vary from board to board

#define MY_IDENTIFIER 0xA1
#define SET_RTC

// global variables and objects
bool usingCRC = false;

RTCZero rtc;
MonitoringNodeData mnd;
Adafruit_ADXL345_Unified adxl;

// functions

void indicateOn() {
    digitalWrite(PIN_STATUSLED, HIGH);
}
void indicateOff() {
    digitalWrite(PIN_STATUSLED, LOW);
}

void setup_mkr1310() {

    rtc.begin();
#if defined(SET_RTC)
    rtc.setEpoch(COMPILE_TIME);
    rtc.disableAlarm();
#endif

#ifdef DEBUG
    Serial.begin(SERIALBAUD);
    while (!Serial && millis() < SERIALTIMEOUT)
        yield();
    Serial.println("Notice: Serial Interface Connected!");
#endif

    pinMode(PIN_LORAMODE, INPUT_PULLUP);
    pinMode(PIN_STATUSLED, OUTPUT);
    pinMode(PIN_ERRORLED, OUTPUT);

    digitalWrite(PIN_STATUSLED, LOW);

    // long mode = (digitalRead(PIN_LORAMODE) == LOW) ? LORA_AMERICA : LORA_AFRICA;
    long mode = LORA_AMERICA;

    if (!LoRa.begin(mode)) {
        Serial.println("Error: LoRa Module Failure");
        while (1)
            ;
    }
    else {
        Serial.println("Notice: LoRa Module Online");
    }

    LoRa.setSpreadingFactor(SPREADFACTOR);
    LoRa.setSignalBandwidth(SIGNALBANDWIDTH);
    LoRa.setSyncWord(SYNCWORD);
    LoRa.setPreambleLength(PREAMBLELEN);
    LoRa.setTxPower(17, PA_OUTPUT_PA_BOOST_PIN); // default 17 is very powerful, trips OCP sometimes. minimum 2
                                                 // not otherwise well documented

#if defined(USING_CRC)
    LoRa.enableCrc();
    usingCRC = true;
#else
    LoRa.disableCrc();
#endif

    mnd.ID = MY_IDENTIFIER;

    switch (mode) {
    case (long) LORA_AMERICA:
        mnd.freq = 0xAC;
        break;
    case (long) LORA_AFRICA:
        mnd.freq = 0xFA;
        break;
    case (long) LORA_EUROPE:
        mnd.freq = 0xEE;
        break;
    default:
        mnd.freq = 0;
    }

    mnd.SyncWord       = SYNCWORD;
    mnd.packetnum      = 0;
    mnd.connectedNodes = 0;
    mnd.timeOnAir      = 0;

#ifdef DEBUG
    if (usingCRC)
        Serial.println("Notice: CRC Enabled");
    else
        Serial.println("Notice: CRC Disabled");
    Serial.println("Notice: Node Setup Complete");
#endif
}
void loop_mkr1310() {
    // whole packet duration in milliseconds
    static const unsigned long toa = (int) ceil(1000 * (PREAMBLELEN + sizeof(MonitoringNodeData) + CRCLEN) *
                                                (0b1 << SPREADFACTOR) / SIGNALBANDWIDTH);

    mnd.status = 0b00000000;
    // all possible status bitshifts TODO
    // mnd.status |= 0b1 << 0;
    // mnd.status |= 0b1 << 1;
    // mnd.status |= 0b1 << 2;
    // mnd.status |= 0b1 << 3;
    // mnd.status |= 0b1 << 4;
    // mnd.status |= 0b1 << 5;
    // mnd.status |= 0b1 << 6;
    // mnd.status |= 0b1 << 7;

    mnd.upTime = millis();
    mnd.epoch  = rtc.getEpoch();
    // analogRead();

    indicateOn();
    unsigned long tst = millis(); // TimeSTart
    mnd.packetnum += 1;

    LoRa.beginPacket();
    LoRa.write((uint8_t *) &mnd, sizeof(MonitoringNodeData));
    LoRa.endPacket(true);

    // notice the rollover check
    while (millis() > tst && millis() - tst < toa)
        ;
    indicateOff();

    // LoRa.sleep();
    Serial.print(mnd.ID);
    Serial.print(" loop ");
    Serial.println(mnd.packetnum);
    delay(5000);
}

#endif
