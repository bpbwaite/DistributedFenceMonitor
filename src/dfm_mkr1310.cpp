/*
  FILE: DFM_MKR1310.CPP
  VERSION: 0.1.2
  DATE: 7 February 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite, Jack Ramsay
  DESCRIPTION: Preliminary test code for MKR1310
*/
#include "dfm_mkr1310.h"
#include "dfm_utils.h"
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
    while (!Serial && millis() < SERIALTIMEOUT)
        yield();
#endif
    Serial.println(F("Notice: Serial Interface Connected!"));

    pinMode(PIN_LORAMODE, INPUT_PULLUP);
    pinMode(PIN_STATUSLED, OUTPUT);
    pinMode(PIN_ERRORLED, OUTPUT);

    indicateOff();

    // long mode = (digitalRead(PIN_LORAMODE) == LOW) ? LORA_AMERICA : LORA_AFRICA;
    long freq = 915E6;

    if (!LoRa.begin(freq)) {
        Serial.println(F("Error: LoRa Module Failure"));
        while (1)
            ;
    }

    Serial.println(F("Notice: LoRa Module Online"));
    Serial.print(F("Notice: Using "));
    Serial.print(freq / 1000000.0, 1);
    Serial.println(F("MHz channel"));
    Serial.print(F("Notice: Max payload "));
    Serial.print(maxPayload());
    Serial.println(F(" Bytes"));
    Serial.print(F("Notice: TOA estimate "));
    Serial.print(getTOA(sizeof(MonitoringNodeData)));
    Serial.println(F(" us"));

    LoRa.setSpreadingFactor(SPREADFACTOR);
    LoRa.setSignalBandwidth(CHIRPBW);
    LoRa.setSyncWord(SYNCWORD);
    LoRa.setPreambleLength(PREAMBLELEN);
    LoRa.setTxPower(15, PA_OUTPUT_PA_BOOST_PIN); // default 17 is very powerful, trips OCP sometimes. minimum 2
                                                 // not otherwise well documented

#if defined(USING_CRC)
    LoRa.enableCrc();
    Serial.println(F("Notice: CRC Enabled"));
#else
    LoRa.disableCrc();
    Serial.println(F("Notice: CRC Disabled"));
#endif

    mnd.ID   = MY_IDENTIFIER;
    mnd.freq = freq;

    mnd.SyncWord       = SYNCWORD;
    mnd.packetnum      = 0;
    mnd.connectedNodes = 0;
    mnd.timeOnAir      = 0;

    Serial.println(F("Notice: Node Setup Complete"));
}
void loop_mkr1310() {

    mnd.status = 0b00000000;

    mnd.upTime = millis();
    mnd.epoch  = rtc.getEpoch();
    mnd.bat    = analogRead(PIN_BATADC);

    indicateOn();
    mnd.packetnum += 1;
    LoRa.beginPacket();
    LoRa.write((uint8_t *) &mnd, sizeof(MonitoringNodeData));
    unsigned long tstart = micros();
    LoRa.endPacket(false); // false to block while sending
    unsigned long toa_measured = micros() - tstart;

    mnd.timeOnAir += getTOA(sizeof(MonitoringNodeData));
    indicateOff();
    Serial.print("measured ");
    Serial.print(toa_measured);
    Serial.print("us\r\n");
    Serial.print("computed ");
    Serial.print(getTOA(sizeof(MonitoringNodeData)));
    Serial.print("us\r\n");

    // LoRa.sleep();
    Serial.print(mnd.ID, HEX);
    Serial.print(" loop ");
    Serial.println(mnd.packetnum);
    delay(5000);
}

#endif
