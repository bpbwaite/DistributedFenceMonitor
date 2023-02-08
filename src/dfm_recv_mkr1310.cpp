/*
  FILE: DFM_MKR1310.CPP
  VERSION: 0.1.4
  DATE: 8 February 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite
  DESCRIPTION: MKR1310 Central Node Code
*/
#include "dfm_mkr1310.h"
#if defined(ARDUINO_SAMD_MKRWAN1310) && defined(CENTRAL_NODE)

#include <Arduino.h>
#include <ArduinoECCX08.h>
#include <LoRa.h>
#include <SPI.h>
#include <time.h>

int counter = 1;

MonitoringNodeData mndBuf;

void epchtostr(char *p, uint32_t epc) {
    struct tm ts;
    time_t now = (time_t) (epc + GMTOFFSET);
    ts         = *localtime(&now);
    strftime(p, 20, "%m-%d-%Y %H:%M:%S", &ts);
}

// pass a buffer of size 150 or greater please
void mndtostr(char *p, const MonitoringNodeData d) {

    PGM_P format = ">ID:     0x%02X\r\n"
                   ">Pack:   %d\r\n"
                   ">Stat:   0x%02X\r\n"
                   ">Cons:   %d\r\n"
                   ">Batt:   %d\r\n"
                   ">Freq:   %02X (%s)\r\n"
                   ">SF:     %d\r\n"
                   ">SW:     0x%02X (%c)\r\n"
                   ">Uptime: %ds\r\n"
                   ">TOA:    %dms\r\n"
                   ">Temp:   %.1f C\r\n"
                   ">Acc X:  %.2fg\r\n"
                   ">Acc Y:  %.2fg\r\n"
                   ">Acc Z:  %.2fg\r\n"
                   ">Epoch:  %s";

    char epochStr[20];
    epchtostr(epochStr, d.epoch);

    char localeStr[9];
    switch (d.freq) {
    case 0xAC:
        strcpy(localeStr, "America");
        break;
    case 0xFA:
        strcpy(localeStr, "Africa");
        break;
    case 0xEE:
        strcpy(localeStr, "Europe");
        break;
    default:
        break;
    }

    char syncWordIsGood[4];
    if (d.SyncWord == SYNCWORD)
        strcpy(syncWordIsGood, "OK");
    else
        strcpy(syncWordIsGood, "ERR");

    sprintf(p,
            format,
            d.ID,
            d.packetnum,
            d.status,
            d.connectedNodes,
            d.bat,
            d.freq,
            localeStr,
            SPREADFACTOR,
            d.SyncWord,
            syncWordIsGood,
            d.upTime / 1000,
            d.timeOnAir,
            d.temperature,
            d.accelX,
            d.accelY,
            d.accelZ,
            epochStr);
}
void setup_recv_mkr1310() {

    Serial.begin(SERIALBAUD);
    while (!Serial && millis() < SERIALTIMEOUT)
        yield();
    Serial.println("Notice: Serial Interface Connected!");

    long mode = LORA_AMERICA;

    if (!LoRa.begin(mode)) {
        Serial.println("Error: LoRa Module Failure");
        while (1)
            ;
    }
    else {
        Serial.println("Notice: LoRa Module Online");
    }

    LoRa.setGain(RECEIVER_GAINMODE);
    LoRa.setSpreadingFactor(SPREADFACTOR);
    LoRa.setSignalBandwidth(SIGNALBANDWIDTH);
    LoRa.setSyncWord(SYNCWORD);
    LoRa.setPreambleLength(PREAMBLELEN);

#if defined(USING_CRC)
    LoRa.enableCrc();
    Serial.println("Notice: CRC Enabled");
#else
    LoRa.disableCrc();
    Serial.println("Notice: CRC Disabled");
#endif

    Serial.println("Notice: Central Node Setup Complete");
}
void loop_recv_mkr1310() {
    static char serialOutBuffer[256];

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

        Serial.print("With RSSI of ");
        Serial.print(LoRa.packetRssi());
        Serial.println(" dBmW");
        Serial.print("With SNR of ");
        Serial.println(LoRa.packetSnr());
        Serial.println();
    }
    delay(10);
}

#endif
