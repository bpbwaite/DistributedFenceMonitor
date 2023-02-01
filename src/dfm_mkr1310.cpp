/*
  FILE: DFM_MKR1310.CPP
  VERSION: 0.0.5
  DATE: 31 January 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite, Jack Ramsay
  DESCRIPTION: Preliminary test code for MKR1310
*/
#include <dfm_mkr1310.h>
#if defined(ARDUINO_SAMD_MKRWAN1310) & !defined(CENTRAL_HUB)

#include <Adafruit_ADXL345_U.h>
#include <Arduino.h>
#include <ArduinoECCX08.h>
#include <ArduinoLowPower.h>
#include <LoRa.h>
#include <RTCZero.h>
#include <SPI.h>

// only defines unique to nodes that vary from board to board

#define MY_IDENTIFIER 0xA1
// #define SET_RTC

// global variables and objects

RTCZero rtc;
MonitoringNodeData mnd;
Adafruit_ADXL345_Unified adxl;

void setup_mkr1310() {

    rtc.begin();
#if defined(SET_RTC)
    rtc.setEpoch(COMPILE_TIME);
    rtc.disableAlarm();
#endif

#ifdef DEBUG
    Serial.begin(SERIALBAUD);
    while (!Serial)
        yield();
    Serial.println("Serial Interface Connected!");
#endif

    pinMode(PIN_LORAMODE, INPUT_PULLUP);
    pinMode(PIN_STATUSLED, INPUT_PULLUP);
    pinMode(PIN_ERRORLED, INPUT_PULLUP);

    long mode = (digitalRead(PIN_LORAMODE) == LOW) ? LORA_AMERICA : LORA_AFRICA;

    if (!LoRa.begin(mode)) {
        Serial.println("LoRa Module Failure");
    }
    else {
        Serial.println("LoRa Module Online");
    }

    LoRa.setSpreadingFactor(SPREADFACTOR);
    LoRa.setSignalBandwidth(SIGNALBANDWIDTH);
    LoRa.setSyncWord(SYNCWORD);
    LoRa.setPreambleLength(PREAMBLELEN);
    // LoRa.setTxPower(17, PA_OUTPUT_PA_BOOST_PIN); // not well documented
#if defined(USING_CRC)
    LoRa.enableCrc();
#else
    LoRa.disableCrc();
#endif

    mnd.ID = MY_IDENTIFIER;
    switch ((int) SIGNALBANDWIDTH) {
    case (int) LORA_AMERICA:
        mnd.freq = 0xAC;
        break;
    case (int) LORA_AFRICA:
        mnd.freq = 0xFA;
        break;
    case (int) LORA_EUROPE:
        mnd.freq = 0xEE;
        break;
    }

    mnd.SF             = SPREADFACTOR;
    mnd.SyncWord       = SYNCWORD;
    mnd.packetnum      = 0;
    mnd.connectedNodes = 0;
    mnd.timeInTransit  = 0;
}
void loop_mkr1310() {

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

    LoRa.beginPacket();
    mnd.packetnum += 1;
    LoRa.write((uint8_t *) &mnd, sizeof(MonitoringNodeData));
    unsigned long tst = millis(); // TimeSTart
    LoRa.endPacket();
    mnd.timeInTransit += (millis() - tst);

    LoRa.sleep();
    delay(1000);
}

#endif
