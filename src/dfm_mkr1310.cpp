/*
  FILE: DFM_MKR1310.CPP
  VERSION: 0.1.9
  DATE: 22 March 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite, Jack Ramsay
  DESCRIPTION: Preliminary test code for MKR1310
*/
#include "dfm_mkr1310.h"
#include "dfm_utils.h"

#if defined(ARDUINO_SAMD_MKRWAN1310) && !defined(CENTRAL_NODE)

#include <Arduino.h>
#include <time.h>

#include <ArduinoECCX08.h>
#include <ArduinoLowPower.h>
#include <Arduino_PMIC.h>
#include <LoRa.h>
#include <RTCZero.h>
#include <SPI.h>
#include <SparkFun_ADXL345.h>
#include <Wire.h>

// only defines unique to nodes that vary from board to board

#define MY_IDENTIFIER 0xB1
#define SET_RTC       true

// global variables and objects
int ADXL_Z_AXIS[ADXL_SAMPLE_LENGTH];
RTCZero rtc;
MND_Compact mnd;
ADXL345 *adxl;

volatile bool motionDetected = false;
void wakeuphandler(void) {
    motionDetected = true;
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
#endif
    Serial.println(F("Notice: Serial Interface Connected!"));

    // ARDUINO PIN INITIALIZATION

    analogReadResolution(10);

    pinMode(PIN_LORAMODE, INPUT_PULLUP);
    pinMode(PIN_SW1, INPUT_PULLUP);
    pinMode(PIN_BATADC, INPUT);
    pinMode(PIN_INTERRUPT, INPUT_PULLDOWN);
    pinMode(PIN_STATUSLED, INPUT);
    pinMode(PIN_ERRORLED, OUTPUT);
    indicateOn();
    errorOff();

    // ADXL INITIALIZATION

    adxl = new ADXL345(PIN_ADXLCS1);

    adxl->powerOn();
    adxl->setSpiBit(0); // 4-Wire SPI
    adxl->setRangeSetting(ADXL_SENSITIVITY);
    adxl->setFullResBit(ADXL_FULLRESBIT);
    adxl->set_bw(ADXL345_BW_50);
    adxl->setInterruptLevelBit(ADXL_RISING); // means the pin RISES on interrupt

    adxl->setActivityAc(1); // AC coupled activitiy
    adxl->setActivityXYZ(0, 0, 1);
    adxl->setActivityThreshold(ADXL_ACT_THRESH);
    adxl->setInactivityAc(1);
    adxl->setInactivityXYZ(0, 0, 1);
    adxl->setInactivityThreshold(ADXL_ACT_THRESH);
    adxl->setTimeInactivity(ADXL_TIME_REST);

    adxl->ActivityINT(1);
    adxl->InactivityINT(0);
    adxl->FreeFallINT(0);
    adxl->doubleTapINT(0);
    adxl->singleTapINT(0);

    // disable FIFO-related interrupts
    adxl->setInterrupt(ADXL345_OVERRUNY, false);
    adxl->setInterrupt(ADXL345_WATERMARK, false);
    adxl->setInterrupt(ADXL345_DATA_READY, false);

    // initial int map
    adxl->setInterruptMapping(ADXL345_ACTIVITY, ADXL345_INT1_PIN);
    adxl->setInterruptMapping(ADXL345_INACTIVITY, ADXL345_INT2_PIN);
    adxl->setInterruptMapping(ADXL345_DATA_READY, ADXL345_INT1_PIN);

    // check for life
    AccelData dum;

    adxl->getInterruptSource();
    adxl->readAccel(&dum.x, &dum.y, &dum.z);
    if ((abs(dum.x) + abs(dum.y) + abs(dum.z)) <= 0) {
        Serial.println(F("Error: ADXL Appears Offline"));
        errorOn();
        while (1)
            ;
    }

    // LORA INITIALIZATION

    // calculate which channel to use first
    // long mode = (digitalRead(PIN_LORAMODE) == LOW) ? LORA_AMERICA : LORA_AFRICA;
    long freq = LoRaChannelsUS[63];

    if (!LoRa.begin(freq)) {
        Serial.println(F("Error: LoRa Module Failure"));
        errorOn();
        while (1)
            ;
    }

    Serial.print(F("Notice: Using "));
    Serial.print(freq / 1000000.0, 1);
    Serial.println(F("MHz channel"));

    LoRa.setSpreadingFactor(SPREADFACTOR);
    LoRa.setSignalBandwidth(CHIRPBW);
    LoRa.setCodingRate4(CODERATE);
    LoRa.setPreambleLength(PREAMBLELEN);
    LoRa.setSyncWord(SYNCWORD);
    LoRa.setTxPower(LORA_POWER, PA_OUTPUT_PA_BOOST_PIN);
    LoRa.setGain(RECEIVER_GAINMODE);

    if (USING_CRC) {
        LoRa.enableCrc();
        Serial.println(F("Notice: CRC Enabled"));
    }
    else {
        LoRa.disableCrc();
        Serial.println(F("Notice: CRC Disabled"));
    }
    Serial.println(F("Notice: LoRa Module Online"));

    // POWER MANAGER INITIALIZATION
    PMIC.begin();
    // here
    PMIC.end();

    // COLLECT STRUCT PARAMETERS

    mnd.ID         = MY_IDENTIFIER;
    mnd.packetnum  = 0;
    mnd.all_states = 0x0F80E402;

    // FINALIZE SETUP

    Serial.println(F("Notice: Node Setup Complete"));
    Serial.println(F("Notice: There will be no more serial messages"));

    // ISR ATTACHMENT
    motionDetected = false;
    LowPower.attachInterruptWakeup(
        PIN_INTERRUPT, wakeuphandler, RISING); // wake up on pin 7 rising edge and attach interrupt to pin 7 and sets
                                               // handler of isr to the function named isr

    indicateOff();

    // if (Serial)
    //     Serial.end();
    // USBDevice.detach();
}

void loop_mkr1310() {
    // execution resumes from sleep here
    rtc.disableAlarm();

    mnd.upTime = millis();
    mnd.epoch  = rtc.getEpoch();

    // convert ADC reading to an estimated percentage and store
    int bat_raw = analogRead(PIN_BATADC);
    setBatt(mnd,
            100 * (((ADC_VREF * (bat_raw) / ((0b1 << ADC_BITS) - 1)) * (R_top + R_bot) / R_bot) - VBAT_ZERO) /
                (VBAT_HUNDRED - VBAT_ZERO));

    // Boolean motionDetected that is changed from the ISR
    if (motionDetected) {
        Serial.println("in the loop");
        motionDetected = false;
        // Data Collection mode
        adxl->setInterrupt(ADXL345_ACTIVITY, false);  // disabling activity interrupt
        adxl->setInterrupt(ADXL345_DATA_READY, true); // enabling data ready interrupt
        // collection logic here
        int i = 0;
        int x, y, z;

        adxl->getInterruptSource();

        while (i < ADXL_SAMPLE_LENGTH) {

            adxl->readAccel(&x, &y, &z);
            ADXL_Z_AXIS[i % ADXL_SAMPLE_LENGTH] = z;

            while (!digitalRead(PIN_INTERRUPT)) // wait for the pin to go high and take sample
                ;
            i++;
        }
        // After Data Collection mode
        adxl->setInterrupt(ADXL345_ACTIVITY, true);    // enabling activity interrupt
        adxl->setInterrupt(ADXL345_DATA_READY, false); // disabling data ready interrupt
        Serial.println("ok loop done");
    }

    Serial.println("awake, send now");

    mnd.packetnum += 1;
    LoRa.beginPacket();
    LoRa.write((uint8_t *) &mnd, sizeof(MND_Compact)); // Write the data to the packet
    LoRa.endPacket(true);                              // false to block while sending
    LoRa.sleep();

    // LowPower.deepSleep(SLEEP_TIME_MS);
    Serial.println("going to bed");
    delay(5000);
}

#endif
