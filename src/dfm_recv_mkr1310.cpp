/*
  FILE: DFM_MKR1310.CPP
  VERSION: 0.1.5
  DATE: 8 February 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite
  DESCRIPTION: MKR1310 Central Node Code
*/
#include "dfm_mkr1310.h"
#include "dfm_utils.h"
#if defined(ARDUINO_SAMD_MKRWAN1310) && defined(CENTRAL_NODE)

#include <Arduino.h>
#include <ArduinoECCX08.h>
#include <LoRa.h>
#include <SPI.h>
#include <time.h>

int counter = 1;

MonitoringNodeData mndBuf;

void setup_recv_mkr1310() {

    Serial.begin(SERIALBAUD);
    while (!Serial && millis() < SERIALTIMEOUT)
        yield();
    Serial.println(F("Notice: Serial Interface Connected!"));

    // long freq = LoRaChannelsUS[63];
    long freq = 915E6;

    if (!LoRa.begin(freq)) {
        Serial.println(F("Error: LoRa Module Failure"));
        while (1)
            ;
    }
    else {
        Serial.println(F("Notice: LoRa Module Online"));
    }

    LoRa.setGain(RECEIVER_GAINMODE);
    LoRa.setSpreadingFactor(SPREADFACTOR);
    LoRa.setSignalBandwidth(CHIRPBW);
    LoRa.setSyncWord(SYNCWORD);
    LoRa.setPreambleLength(PREAMBLELEN);

#if defined(USING_CRC)
    LoRa.enableCrc();
    Serial.println(F("Notice: CRC Enabled"));
#else
    LoRa.disableCrc();
    Serial.println(F("Notice: CRC Disabled"));
#endif

    Serial.println(F("Notice: Central Node Setup Complete"));
}
void loop_recv_mkr1310() {
    char serialOutBuffer[512];

    int packetSize = LoRa.parsePacket();
    if (packetSize) {

        Serial.print("RECEIVED: #");
        Serial.println(counter++);

        int byteIndexer = 0;
        while (LoRa.available() && byteIndexer < sizeof(MonitoringNodeData)) {
            ((uint8_t *) &mndBuf)[byteIndexer++] = (uint8_t) LoRa.read();
        }

        mndtostr(serialOutBuffer, mndBuf);
        Serial.println(serialOutBuffer);
    }
    delay(10);
}

#endif
