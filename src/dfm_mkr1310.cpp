/*
  FILE: DFM_MKR1310.CPP
  VERSION: 0.2.5
  DATE: 27 March 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite, Jack Ramsay, Renzo Mena, Mike Paff
  DESCRIPTION: Preliminary test code for MKR1310
*/
#include "dfm_mkr1310.h"
#include "dfm_errors.h"
#include "dfm_utils.h"

#if defined(ARDUINO_SAMD_MKRWAN1310) && !defined(CENTRAL_NODE)

#include <Arduino.h>
#include <time.h>

#include <ArduinoECCX08.h>
#include <ArduinoLowPower.h>
#include <Arduino_PMIC.h>
#include <LoRa.h>
#include <RTCZero.h>
// #include <WDTZero.h>
#include <SPI.h>
#include <SparkFun_ADXL345.h>
#include <Wire.h>

// only defines unique to nodes that vary from board to board

#define MY_IDENTIFIER 0xB1
#define SET_RTC       true

// global variables and objects
RTCZero rtc;
MND_Compact mnd;
ADXL345 *adxl;

// Data collection items

int DC_offset = 0;
double Z_Power_Samples[ADXL_SAMPLE_LENGTH];
int severityLevel = 0;

// isr related items
volatile bool motionDetected = false;
volatile bool need_reattach  = false;

void wakeuphandler(void) {
    motionDetected = true;
}

// timing related items

volatile unsigned long TOLR = 0; // time of last real-time-clock setting
inline unsigned long TSLR() {
    return millis() - TOLR;
} // time since last calibration
volatile unsigned long TOLC = 0; // time of last calibration
inline unsigned long TSLC() {
    return millis() - TOLC;
} // time since last calibration
volatile unsigned long TOLS = 0; // time of last sample
inline unsigned long TSLS() {
    return millis() - TOLS;
} // time since last sample

void setup_mkr1310() {

    // ARDUINO PIN INITIALIZATION

    analogReadResolution(ADC_BITS);

    pinMode(PIN_LORAMODE, INPUT_PULLUP);
    pinMode(PIN_SW1, INPUT_PULLUP);
    pinMode(PIN_BATADC, INPUT);
    pinMode(PIN_INTERRUPT, INPUT_PULLDOWN);
    pinMode(PIN_STATUSLED, INPUT);
    pinMode(PIN_ERRORLED, OUTPUT);

    indicateOn();
    errorOff();

    // RTC

    rtc.begin();
    if (SET_RTC) {
        rtc.setEpoch(COMPILE_TIME);
        rtc.disableAlarm();
    }

    if (DEBUG) {
        Serial.begin(SERIALBAUD);
        while (!Serial && millis() < SERIALTIMEOUT)
            yield();
    }
    Serial.println(F("Serial Interface Connected!"));

    // CONFIGURE WATCHDOG TIMER IF APPLICABLE
    // wdt.setup(WDT_HARDCYCLE8S);
    // wdt.clear();

    // ADXL INITIALIZATION

    while (1) { // may take multiple attempts

        adxl = new ADXL345(PIN_ADXLCS1);

        fullResetADXL(adxl);

        // check for life
        AccelData dum;
        adxl->getInterruptSource();
        adxl->readAccel(&dum.x, &dum.y, &dum.z);
        if ((abs(dum.x) + abs(dum.y) + abs(dum.z)) <= 0) {
            Serial.println(F("Error: ADXL Appears Offline"));
            ERROR_OUT(ERROR_ADXL345);
        }
        else
            break;
    }
    adxlMode(adxl, ADXL_MOTION);

    // LORA INITIALIZATION

    // calculate which channel to use first
    // the final product should determine this by other methods
    long freq;
    if (digitalRead(PIN_LORAMODE)) {
        freq = LoRaChannelsUS[63];
    }
    else {
        freq = LoRaChannelsEU[7];
    }
    Serial.print(F("Using "));
    Serial.print(freq / 1000000.0, 1);
    Serial.println(F("MHz channel"));

    if (!LoRa.begin(freq)) {
        Serial.println(F("Error: LoRa Module Failure"));
        ERROR_OUT(ERROR_NO_LORA);
    }

    LoRa.setSpreadingFactor(SPREADFACTOR);
    LoRa.setSignalBandwidth(CHIRPBW);
    LoRa.setCodingRate4(CODERATE);
    LoRa.setPreambleLength(PREAMBLELEN);
    LoRa.setSyncWord(SYNCWORD);
    LoRa.setTxPower(LORA_POWER, PA_OUTPUT_PA_BOOST_PIN);
    LoRa.setGain(RECEIVER_GAINMODE);

    if (USING_CRC) {
        LoRa.enableCrc();
        Serial.println(F("CRC Enabled"));
    }
    else {
        LoRa.disableCrc();
        Serial.println(F("CRC Disabled"));
    }
    Serial.println(F("LoRa Module Operational"));

    // POWER MANAGER INITIALIZATION

    PMIC.begin();
    // here
    PMIC.end();

    // COLLECT SOME STRUCT PARAMETERS

    mnd.ID         = MY_IDENTIFIER;
    mnd.packetnum  = 0;
    mnd.all_states = 0x0F80E402;

    // CALIBRATE FOR BIAS
    DC_offset = getDCOffset(adxl, CALIBRATION_TIME_SLICE);
    TOLC      = millis();

    // ISR ATTACHMENT
    motionDetected = false;
    LowPower.attachInterruptWakeup(PIN_INTERRUPT, wakeuphandler, RISING); // wake up on pin 7 rising

    // FINALIZE
    indicateOff();
    Serial.println(F("Node Setup Complete"));

    if (!DEBUG) {
        if (Serial)
            Serial.end();

        USBDevice.detach();
    }
}

void loop_mkr1310() {
    // execution resumes from sleep here
    rtc.disableAlarm();

    // Boolean motionDetected that is changed from the ISR
    if (motionDetected) {
        // collection logic variables
        int i = 0;
        int x, y, z;

        adxl->readAccel(&x, &y, &z);
        Serial.print("Z=");
        Serial.print(z);
        Serial.print("/");
        Serial.println(DC_offset);

        Serial.println(F("Wakeup was due to motion"));
        motionDetected = false;
        // Data Collection mode
        detachInterrupt(digitalPinToInterrupt(PIN_INTERRUPT));
        need_reattach = true;

        // collect data:
        adxlMode(adxl, ADXL_COLLECTION);
        adxl->getInterruptSource();
        while (i < ADXL_SAMPLE_LENGTH) {
            digitalWrite(PIN_STATUSLED, !digitalRead(PIN_STATUSLED));

            adxl->readAccel(&x, &y, &z);
            Z_Power_Samples[i] = sq((z - DC_offset) * GRAVITY / (double) ADXL_LSB_PER_G_Z);

            TOLS = millis();

            while (!digitalRead(PIN_INTERRUPT) && (TSLS() < 500UL))
                // wait for the pin to go high and take sample
                // a 1/2 second watchdog is implemented here
                ;

            i++;
        }

        // Renzo's algorithm

        double currentMax = 0;
        severityLevel     = 0;

        for (i = 0; i < ADXL_SAMPLE_LENGTH; ++i) {
            if (Z_Power_Samples[i] > currentMax) {
                // this will be checking for the current max energy
                currentMax = Z_Power_Samples[i];
            }
        }
        Serial.print("Max of ");
        Serial.println(currentMax);
        //  following for-loop should loop until thresholdZ is no longer passed
        for (i = 0; i < 15; ++i) {
            if (currentMax < thresholdZ_logarithmic[i])
                break;
            severityLevel++;
        }

        Serial.print("severity determined to be ");
        Serial.println(severityLevel);

        Serial.println("exit motion loop");
    }

    errorOn();
    // get basic status indicators
    mnd.packetnum += 1;
    mnd.upTime = millis();
    mnd.epoch  = rtc.getEpoch();

    // compact-byte types:
    setSeverity(mnd, severityLevel);
    severityLevel = 0; // and viola, reset

    setTSLC(mnd, TSLC() / 60000); // convert ms to minutes;

    setTemperature(mnd, 25);

    AccelData dum;
    adxl->getInterruptSource();
    adxl->readAccel(&dum.x, &dum.y, &dum.z);
    setIMUBit(mnd, ((abs(dum.x) + abs(dum.y) + abs(dum.z)) > 0));

    int bat_raw = analogRead(PIN_BATADC);
    // convert ADC reading to an estimated percentage and store
    setBatt(mnd,
            100 * (((ADC_VREF * (bat_raw) / ((0b1 << ADC_BITS) - 1)) * (R_top + R_bot) / R_bot) - VBAT_ZERO) /
                (VBAT_HUNDRED - VBAT_ZERO));

    setConnections(mnd, 1);

    while (rtc.getEpoch() % 15 != 0) {
        if (rtc.getEpoch() % 15 == 0) {
            break;
        }
    }

    // WAIT UNTIL TIME TO SEND IS UPON US
    // while (rtc.getEpoch() % (SLEEP_TIME_MS / 1000) != 0) {
    //    ; // ?
    //}
    // send status indicators
    LoRa.beginPacket();
    LoRa.write((uint8_t *) &mnd, sizeof(MND_Compact)); // Write the data to the packet
    LoRa.endPacket(false);                             // false to block while sending
    LoRa.sleep();
    errorOff();

    // check if we should recompute the DC offset
    if (TSLC() > ADXL_CALIBRATION_INTERVAL) {
        DC_offset     = getDCOffset(adxl, CALIBRATION_TIME_SLICE);
        need_reattach = true; // after the offset you need this
        TOLC          = millis();
    }

    // the mode was also changed to data collection. instead,
    // wait for settle
    adxlMode(adxl, ADXL_SETTLING);

    while (!digitalRead(PIN_INTERRUPT))
        ; // wait for inactivity pin to go high

    adxlMode(adxl, ADXL_MOTION);

    if (need_reattach) {
        LowPower.attachInterruptWakeup(PIN_INTERRUPT, wakeuphandler, RISING);
        need_reattach = false;
    }

    adxl->getInterruptSource();

    // should we sleep, or just wait for the next period?
    int next_wake_epoch;
    next_wake_epoch = rtc.getEpoch() + (SLEEP_TIME_MS / 1000);
    while (next_wake_epoch % (SLEEP_TIME_MS / 1000) != 0)
        next_wake_epoch--;

    // LowPower.deepSleep(SLEEP_TIME_MS);

    // simulated sleeping:
    motionDetected = false; // attaching an interrupt may cause it to run. unset it here.
    while (!digitalRead(PIN_INTERRUPT) && rtc.getEpoch() < next_wake_epoch)
        ;
}

#endif
