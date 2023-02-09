/*
  FILE: DFM_MKR1310.CPP
  VERSION: 0.1.5
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
    strftime(p, 22, "%m-%d-%Y %H:%M:%S", &ts);
}

void mndtostr(char *p, const MonitoringNodeData d) {

    char channelStr[20];
    int k;
    if (d.freq >= LoRaChannelsUS[0] && d.freq <= LoRaChannelsUS[NUMCHANNELS_US]) {
        for (k = 0; k < NUMCHANNELS_US; ++k)
            if (LoRaChannelsUS[k] == d.freq)
                break;
        sprintf(channelStr, "America Ch.%d", k);
    }
    else if (d.freq >= LoRaChannelsEU[0] && d.freq <= LoRaChannelsEU[NUMCHANNELS_EU]) {
        int k;
        for (k = 0; k < NUMCHANNELS_EU; ++k)
            if (LoRaChannelsEU[k] == d.freq)
                break;
        sprintf(channelStr, "Europe Ch.%d", k);
    }
    else
        strcpy(channelStr, "Unknown");

    char syncWordGoodStr[4];
    if (d.SyncWord == SYNCWORD)
        strcpy(syncWordGoodStr, "OK");
    else
        strcpy(syncWordGoodStr, "ERR");

    char epochStr[22];
    epchtostr(epochStr, d.epoch);

    char confStr[10];
    sprintf(confStr, "SF%dBW%d", SPREADFACTOR, int(CHIRPBW / 1000U));

    char rangeStr[6];
    int rssi = LoRa.packetRssi();
    int dist = 0;
    if (rssi >= -18)
        dist = 0;
    else if (rssi >= -28)
        dist = 1;
    else if (rssi >= -48)
        dist = 5;
    else if (rssi >= -55)
        dist = 10;
    else if (rssi >= -60)
        dist = 50;
    else
        dist = 5000;

    sprintf(rangeStr, "~%d m", dist);

    // snr relative strength is SF dependent
    float snr = LoRa.packetSnr();

    PGM_P format = ">ID:     0x%02X\r\n"
                   ">Pack:   %d\r\n"
                   ">Stat:   0x%02X\r\n"
                   ">Cons:   %d\r\n"
                   ">Batt:   %d\r\n"
                   ">Freq:   %.1fMHz (%s)\r\n"
                   ">SW:     0x%04X (%s)\r\n"
                   ">Uptime: %ds\r\n"
                   ">TOA:    %dms\r\n"
                   ">Temp:   %.1fC\r\n"
                   ">Acc X:  %.2fg\r\n"
                   ">Acc Y:  %.2fg\r\n"
                   ">Acc Z:  %.2fg\r\n"
                   ">Epoch:  %s\r\n"
                   ">Conf:   %s\r\n"
                   ">RSSI:   %ddBmW (%s)\r\n"
                   ">SNR:    %.1fdB";

    sprintf(p,
            format,
            d.ID,
            d.packetnum,
            d.status,
            d.connectedNodes,
            d.bat,
            d.freq / 1000000.0,
            channelStr,
            d.SyncWord,
            syncWordGoodStr,
            d.upTime / 1000,
            d.timeOnAir,
            d.temperature,
            d.accelX,
            d.accelY,
            d.accelZ,
            epochStr,
            confStr,
            rssi,
            rangeStr,
            snr);
}
void setup_recv_mkr1310() {

    Serial.begin(SERIALBAUD);
    while (!Serial && millis() < SERIALTIMEOUT)
        yield();
    Serial.println("Notice: Serial Interface Connected!");

    // long freq = LoRaChannelsUS[63];
    long freq = 915E6;

    if (!LoRa.begin(freq)) {
        Serial.println("Error: LoRa Module Failure");
        while (1)
            ;
    }
    else {
        Serial.println("Notice: LoRa Module Online");
    }

    LoRa.setGain(RECEIVER_GAINMODE);
    LoRa.setSpreadingFactor(SPREADFACTOR);
    LoRa.setSignalBandwidth(CHIRPBW);
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
