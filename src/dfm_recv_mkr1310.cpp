/*
  FILE: DFM_MKR1310.CPP
  VERSION: 0.1.0
  DATE: 7 February 2023
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

bool usingCRC = false;
int counter   = 1;

MonitoringNodeData mndBuf;

// todo: don't use WString class (memory fragmentation issues)

String epochToDate(uint32_t epc) {
    struct tm ts;
    char buf[80];
    time_t now = (time_t) (epc + GMTOFFSET);

    ts = *localtime(&now);
    strftime(buf, sizeof(buf), "%m-%d-%Y %H:%M:%S", &ts);
    return String(buf);
}

String MND_toString(const MonitoringNodeData d) {
    static const char dlm[3]  = ": ";
    static const char crlf[3] = "\r\n";
    char itoa_buf[33];
    String s = "";

    s += F("ID    ");
    s += dlm;
    s += itoa(d.ID, itoa_buf, HEX);
    s += crlf;
    s += F("Pack  ");
    s += dlm;
    s += d.packetnum;
    s += crlf;
    s += F("Stat  ");
    s += dlm;
    s += itoa(d.status, itoa_buf, HEX);
    s += crlf;
    s += F("Cons  ");
    s += dlm;
    s += itoa(d.connectedNodes, itoa_buf, DEC);
    s += crlf;
    s += F("Batt  ");
    s += dlm;
    s += itoa(d.bat, itoa_buf, DEC);
    s += crlf;
    s += F("Freq  ");
    s += dlm;
    s += itoa(d.freq, itoa_buf, HEX);
    s += crlf;
    s += F("SF    ");
    s += dlm;
    s += SPREADFACTOR;
    s += crlf;
    s += F("SW    ");
    s += dlm;
    s += itoa(d.SyncWord, itoa_buf, HEX);
    s += crlf;
    s += F("Uptim ");
    s += dlm;
    s += itoa(d.upTime / 1000, itoa_buf, DEC);
    s += "s";
    s += crlf;
    s += F("TOA   ");
    s += dlm;
    s += itoa(d.timeOnAir, itoa_buf, DEC);
    s += "ms";
    s += crlf;
    s += F("Temp  ");
    s += dlm;
    s += d.temperature;
    s += crlf;
    s += F("Epoch ");
    s += dlm;
    s += epochToDate(d.epoch);

    return s;
}
void setup_recv_mkr1310() {

    Serial.begin(SERIALBAUD);
    while (!Serial)
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
    usingCRC = true;
#else
    LoRa.disableCrc();
#endif

    if (usingCRC)
        Serial.println("Notice: CRC Enabled");
    else
        Serial.println("Notice: CRC Disabled");

    Serial.println("Notice: Central Node Setup Complete");
}
void loop_recv_mkr1310() {

    int packetSize = LoRa.parsePacket();
    if (packetSize) {

        Serial.print("RECEIVED: #");
        Serial.println(counter++);

        int byteIndexer = 0;
        while (LoRa.available() && byteIndexer < sizeof(MonitoringNodeData)) {
            ((uint8_t *) &mndBuf)[byteIndexer++] = (uint8_t) LoRa.read();
        }

        Serial.println(MND_toString(mndBuf));
        Serial.print("With RSSI of ");
        Serial.print(LoRa.packetRssi());
        Serial.println(" dB");
        Serial.print("With SNR of ");
        Serial.println(LoRa.packetSnr());
        Serial.println();
    }
}

#endif
