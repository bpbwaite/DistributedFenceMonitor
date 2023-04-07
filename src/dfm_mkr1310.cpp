/*
  FILE: DFM_MKR1310.CPP
  VERSION: 0.9.0
  DATE: 7 April 2023
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

#define MY_IDENTIFIER 0x01 // 0-13; small scale build ID determines turn
#define SET_RTC       true

// global variables and objects
RTCZero rtc;
MND_Compact mnd;
ADXL345 *adxl;

// Data collection items
int attackCounter = 0;

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

uint32_t next_wake_epoch = 0;
uint32_t next_send_epoch = 0;
// init to zero so it is seen as past time to send

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
    showtime();
    Serial.println(F("Serial Interface Connected!"));

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
    showtime();
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

    showtime();
    if (USING_CRC) {
        LoRa.enableCrc();
        Serial.print(F("CRC Enabled: "));
    }
    else {
        LoRa.disableCrc();
        Serial.print(F("CRC Disabled: "));
    }
    Serial.println(F("LoRa Module Operational"));

    // POWER MANAGER INITIALIZATION

    PMIC.begin();
    // here
    PMIC.end();
    showtime();
    Serial.println(F("Power Manager IC Online"));

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
    showtime();
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
    showtime();
    Serial.println("Wokege");

    attackCounter = 0;
    severityLevel = 0;
    // Boolean motionDetected that is changed from the ISR
    if (motionDetected) {

        showtime();
        Serial.print(F("Wakeup was due to motion ("));

        // collection logic variables
        int x, y, z;

        adxl->readAccel(&x, &y, &z);
        Serial.print("Z=");
        Serial.print(z);
        Serial.print("/");
        Serial.print(DC_offset);
        Serial.println(")");

        motionDetected = false;
        // Data Collection mode
        detachInterrupt(digitalPinToInterrupt(PIN_INTERRUPT));
        need_reattach = true;

        while (++attackCounter <= 2) { // make this 2 a define please
            // collect data:
            int i = 0;
            adxlMode(adxl, ADXL_COLLECTION);
            adxl->getInterruptSource();
            while (i < ADXL_SAMPLE_LENGTH) {
                digitalWrite(PIN_ERRORLED, !digitalRead(PIN_ERRORLED));

                adxl->readAccel(&x, &y, &z);
                Z_Power_Samples[i] = sq((z - DC_offset) * GRAVITY / (double) ADXL_LSB_PER_G_Z);

                TOLS = millis();

                while (!digitalRead(PIN_INTERRUPT) && (TSLS() < ADXL_SAMPLE_TIMEOUT))
                    // wait for the pin to go high and take sample
                    ;

                i++;
            }

            // Renzo's algorithm
            double currentMax     = 0;
            int periodic_severity = 0;
            for (i = 0; i < ADXL_SAMPLE_LENGTH; ++i) {
                if (Z_Power_Samples[i] > currentMax) {
                    // this will be checking for the current max energy
                    currentMax = Z_Power_Samples[i];
                }
            }
            //  following for-loop should loop until thresholdZ is no longer passed
            for (i = 0; i < 15; ++i) {
                if (currentMax < thresholdZ_logarithmic[i])
                    break;
                periodic_severity++;
            }

            severityLevel = max(severityLevel, periodic_severity);

            if (inactivityInDataEnd(&i, 1, 2, 3)) {
                showtime();
                Serial.println(F("No need to scan again"));
                break;
            }
            else {
                showtime();
                Serial.println(F("Still some movement, continuing scan"));
            }
        }

        showtime();
        Serial.print("Peak severity of ");
        Serial.print(severityLevel);
        Serial.println(". Exit.");
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

    // WAIT UNTIL TIME TO SEND IS UPON US

    next_send_epoch = rtc.getEpoch() + (SLEEP_TIME_MS / 1000);
    while (next_send_epoch % (SLEEP_TIME_MS / 1000) != 0)
        next_send_epoch--;
    next_send_epoch += MY_IDENTIFIER;

    showtime();
    Serial.println(F("Waiting for my turn..."));
    while (rtc.getEpoch() < next_send_epoch) {
        yield(); // ? chilling?
    }

    // show what we are sending
    showtime();
    Serial.print(F("Sending Data: 0x"));
    for (int k = 0; k < sizeof(MND_Compact); ++k) {
        Serial.print(((uint8_t *) &mnd)[k], 16);
    }
    Serial.println();

    // send status indicators
    LoRa.beginPacket();
    LoRa.write((uint8_t *) &mnd, sizeof(MND_Compact)); // Write the data to the packet
    LoRa.endPacket(false);                             // false to block while sending
    LoRa.sleep();

    showtime();
    Serial.println(F("Sent!"));
    errorOff();

    // check if we should recompute the DC offset
    if (TSLC() > ADXL_CALIBRATION_INTERVAL) {
        DC_offset     = getDCOffset(adxl, CALIBRATION_TIME_SLICE);
        need_reattach = true; // after the offset you need this
        TOLC          = millis();
    }

    adxlMode(adxl, ADXL_MOTION);

    if (need_reattach) {
        LowPower.attachInterruptWakeup(PIN_INTERRUPT, wakeuphandler, RISING);
        need_reattach = false;
    }

    adxl->getInterruptSource();
    motionDetected = false; // attaching an interrupt may cause it to run. unset it here.

    // find out when to set my alarm for
    next_wake_epoch = rtc.getEpoch() + (SLEEP_TIME_MS / 1000);
    while (next_wake_epoch % (SLEEP_TIME_MS / 1000) != 0)
        next_wake_epoch--;

    next_wake_epoch -= 1;
    // need to be awake BY the epoch, so wake up 1s before it
    // to complete any other computations

    showtime();
    Serial.print("I shleep till ");
    Serial.println(next_wake_epoch);

    // LowPower.deepSleep(next_wake_epoch - rtc.getEpoch());
    // simulated sleeping:
    while (!digitalRead(PIN_INTERRUPT) && rtc.getEpoch() < next_wake_epoch)
        ;
}

#endif
