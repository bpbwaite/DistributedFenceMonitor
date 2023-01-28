/*
  FILE: DFM_MKR1310.CPP
  VERSION: 0.0.4
  DATE: 27 January 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite, Jack Ramsay
  DESCRIPTION: Preliminary test code for MKR1310
*/
#include <dfm_mkr1310.h>
#if defined(ARDUINO_SAMD_MKRWAN1310) & !defined(CENTRAL_HUB)

#include <Arduino.h>
#include <ArduinoECCX08.h>
#include <ArduinoLowPower.h>
#include <LoRa.h>
#include <RTCZero.h>
#include <SPI.h>

// only compiler defines that vary from board to board

#define MY_IDENTIFIER 0xA1
// #define SET_RTC

// global variables and objects

RTCZero rtc;
MonitoringNodeData mnd;

void setup_mkr1310() {

    rtc.begin();
#if defined(SET_RTC)
    rtc.setEpoch(COMPILE_TIME);
    rtc.disableAlarm();
#endif

#ifdef DEBUG
    Serial.begin(SERIALBAUD);
    while (!Serial)
        ;
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

    // LoRa.setGain(0); // Range 1-6. 0 is automatic. Applies to receiver only
    LoRa.setSpreadingFactor(SPREADFACTOR);
    LoRa.setSignalBandwidth(SIGNALBANDWIDTH);
    LoRa.setSyncWord(SYNCWORD);
    LoRa.setPreambleLength(PREAMBLELEN);
    LoRa.enableCrc(); // must match receiver
    // LoRa.setTxPower(17, PA_OUTPUT_PA_BOOST_PIN); // not well documented

    mnd.ID             = MY_IDENTIFIER;
    mnd.freq           = SIGNALBANDWIDTH;
    mnd.SF             = SPREADFACTOR;
    mnd.SyncWord       = SYNCWORD;
    mnd.packetnum      = 0;
    mnd.connectedNodes = 0;
    mnd.timeInTransit  = 0;
}
void loop_mkr1310() {

    mnd.status = 0x1; // temp value
    mnd.upTime = millis();
    mnd.epoch  = rtc.getEpoch();

    unsigned long tst = millis(); // TimeSTart
    LoRa.beginPacket();
    mnd.packetnum += 1;
    LoRa.write((uint8_t *) &mnd, sizeof(MonitoringNodeData));
    LoRa.endPacket();
    mnd.timeInTransit += (millis() - tst);
    // LoRa.sleep();
}

#endif
