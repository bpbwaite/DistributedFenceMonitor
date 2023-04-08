/*
  FILE: DFM_MKR1310.CPP
  VERSION: 1.0.0
  DATE: 27 March 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite
  DESCRIPTION: MKR1310 Central Node Code
*/
#include "dfm_errors.h"
#include "dfm_mkr1310.h"
#include "dfm_utils.h"

#if defined(ARDUINO_SAMD_MKRWAN1310) && defined(CENTRAL_NODE)

#include <Arduino.h>
#include <ArduinoECCX08.h>
#include <ArduinoLowPower.h>
#include <LoRa.h>
#include <SPI.h>
#include <time.h>

#define SET_RTC true

// global variables and objects

RTCZero rtc;
MND_Compact mnd_received;
MND_Ack mnd_acknowledgement;
MND_Report mnd_printable;

long freq                  = LoRaChannelsUS[63];
volatile int interruptFlag = 0;

void setup_recv_mkr1310() {

    rtc.begin();
    if (SET_RTC) {
        rtc.setEpoch(COMPILE_TIME);
        rtc.disableAlarm();
    }

    Serial.begin(SERIALBAUD);
    while (!Serial && millis() < SERIALTIMEOUT)
        yield();
    if (!Serial)
        ERROR_OUT(ERROR_NO_SERIAL);

    Serial.println(F("Notice: Serial Interface Connected!"));

    pinMode(PIN_STATUSLED, OUTPUT);
    pinMode(LORA_IRQ, INPUT);
    indicateOff();

    if (!LoRa.begin(freq)) {
        Serial.println(F("Error: LoRa Module Failure"));
        while (1)
            ERROR_OUT(ERROR_NO_LORA);
    }
    Serial.println(F("Notice: LoRa Module Online"));

    LoRa.setGain(RECEIVER_GAINMODE);
    LoRa.setSpreadingFactor(SPREADFACTOR);
    LoRa.setSignalBandwidth(CHIRPBW);
    LoRa.setCodingRate4(CODERATE);
    LoRa.setSyncWord(SYNCWORD);
    LoRa.setPreambleLength(PREAMBLELEN);
    LoRa.setTxPower(12, PA_OUTPUT_PA_BOOST_PIN);

    if (USING_CRC) {
        LoRa.enableCrc();
        Serial.println(F("Notice: CRC Enabled"));
    }
    else {
        LoRa.disableCrc();
        Serial.println(F("Notice: CRC Disabled"));
    }

    // Lora ACK packet parameters
    mnd_acknowledgement.universal_epoch = rtc.getEpoch();
    mnd_acknowledgement.universal_millis =
        millis(); // TODO define this value as the remainder of millis at which rtc ticks occur
    mnd_acknowledgement.weak_signal_please_increase = false;

    // SPI.usingInterrupt(digitalPinToInterrupt(LORA_IRQ));
    // SPI.notUsingInterrupt(digitalPinToInterrupt(LORA_IRQ));
    //  need to call this if you want to reset the internal IVT over SPI

    //  interrupt attachment order matters
    LowPower.attachInterruptWakeup(
        digitalPinToInterrupt(LORA_IRQ), []() -> void {}, RISING);
    LoRa.onReceive([](int sz) -> void { interruptFlag = sz; });
    Serial.println(F("Notice: Callback function bound to receiver"));

    Serial.println(F("Notice: Central Node Setup Complete"));
}
void loop_recv_mkr1310() {

    indicateOff();

    LoRa.receive();
    while (!interruptFlag)
        ;

    indicateOn();

    int byteIndexer = 0;
    while (LoRa.available() && byteIndexer < sizeof(MND_Compact))
        ((uint8_t *) &mnd_received)[byteIndexer++] = (uint8_t) LoRa.read();

    delay(250);

    // respond with acknowledgement
    mnd_acknowledgement.universal_epoch             = rtc.getEpoch();
    mnd_printable.rssi                              = LoRa.packetRssi();
    mnd_acknowledgement.weak_signal_please_increase = (mnd_printable.rssi < WEAK_RSSI) ? 1 : 0;

    LoRa.beginPacket();
    LoRa.write((uint8_t *) &mnd_acknowledgement, sizeof(MND_Ack)); // Write the data to the packet
    LoRa.endPacket(false);

    // expand data for easier processing
    mnd_printable.ID             = mnd_received.ID;
    mnd_printable.packetnum      = mnd_received.packetnum;
    mnd_printable.temperature    = getTemperature(mnd_received);
    mnd_printable.bat            = getBatt(mnd_received);
    mnd_printable.severity       = getSeverity(mnd_received);
    mnd_printable.connectedNodes = getConnections(mnd_received);
    mnd_printable.hasAccel       = getIMUBit(mnd_received);
    mnd_printable.wantsRTC       = getNeedRTC(mnd_received);
    mnd_printable.upTime         = mnd_received.upTime;
    mnd_printable.epoch          = mnd_received.epoch;
    mnd_printable.minSinceCal    = getTSLC(mnd_received);
    mnd_printable.sw             = SYNCWORD;
    mnd_printable.bw             = CHIRPBW;
    mnd_printable.freq           = LoRaChannelsUS[63];
    mnd_printable.hops           = 0;

    Serial.print('#');
    for (int nb = 0; nb < sizeof(MND_Report); ++nb)
        Serial.write((unsigned char) ((uint8_t *) &mnd_printable)[nb]);
    Serial.println();

    // prepare for next packet
    interruptFlag = 0;
}

#endif
