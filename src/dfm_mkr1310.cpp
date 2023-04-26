/*
  FILE: DFM_MKR1310.CPP
  VERSION: 1.0.0
  DATE: 16 April 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite, Jack Ramsay, Renzo Mena, Mike Paff
  DESCRIPTION: MKR1310 Node Code
*/
#include "dfm_mkr1310.h"
#include "dfm_errors.h"
#include "dfm_utils.h"

#if defined(ARDUINO_SAMD_MKRWAN1310) && !defined(CENTRAL_NODE)

#include <Arduino.h>
// do not sort includes!
// clang-format off
#include <Wire.h>
#include <SPI.h>
#include <RTCZero.h>
#include <ArduinoLowPower.h>
#include <Arduino_PMIC.h>
#include <LoRa.h>
#include <SparkFun_ADXL345.h>
#include <DHT.h>
// clang-format on

// only defines unique to nodes that vary from board to board

#define MY_IDENTIFIER 0x02 // 0-13; small scale build ID determines turn
#define SET_RTC       true

// global variables and objects
RTCZero rtc;
MND_Compact mnd;
MND_Ack mnd_response;
ADXL345 *adxl;
DHT *tSensor;

// System Counters

int adxlAttempts  = 0; // num times attempted to communicate with sensor
int attackCounter = 0;
uint32_t central_milliseconds;
uint32_t ms_offset;       // TODO compute and use ms offset to more accurately schedule wake
volatile int ackFlag = 0; // bytes received in acknowledgement

// Data collection items

int DC_offset = 0;
double Z_Power_Samples[ADXL_SAMPLE_LENGTH];
int severityLevel = 0;
double LPF_FIR[FIRSIZE];

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
volatile unsigned long TOLA = 0; // time of last ack
inline unsigned long TSLA() {
    return millis() - TOLA;
} // time since last ack

void setup_mkr1310() {

    // ARDUINO PIN INITIALIZATION

    analogReadResolution(ADC_BITS);

    pinMode(PIN_LORAMODE, INPUT_PULLUP);
    pinMode(PIN_SW1, INPUT_PULLUP);
    pinMode(PIN_BATADC, INPUT);
    pinMode(PIN_INTERRUPT, INPUT_PULLDOWN);
    pinMode(PIN_STATUSLED, INPUT);
    pinMode(PIN_ERRORLED, OUTPUT);
    pinMode(LORA_IRQ, INPUT);

    pinMode(PIN_DHT, OUTPUT);

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
    timeStamp();
    Serial.println(F("Serial Interface Connected!"));

    // ADXL INITIALIZATION

    while (++adxlAttempts <= ADXL_CONNECTION_TIMEOUT) { // may take 2+ attempts

        adxl = new ADXL345(PIN_ADXLCS1);

        fullResetADXL(adxl);

        // check for life
        AccelData dum;
        adxl->getInterruptSource();
        adxl->readAccel(&dum.x, &dum.y, &dum.z);
        if ((abs(dum.x) + abs(dum.y) + abs(dum.z)) <= 0) {
            Serial.println(F("No ADXL on SPIBUS, retrying..."));
            delay(10);
        }
        else
            break;
    }
    if (adxlAttempts > ADXL_CONNECTION_TIMEOUT) {
        Serial.println(F("Error: ADXL Appears Offline"));
        // while (1)
        //     ERROR_OUT(ERROR_ADXL345);
        //  todo: more gracefully handle (ex. don't run calibration)
    }
    adxlMode(adxl, ADXL_MOTION);

    timeStamp();
    Serial.println(F("ADXL Online!"));

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

    timeStamp();
    Serial.print(F("Using "));
    Serial.print(freq / 1000000.0, 1);
    Serial.println(F("MHz channel"));

    if (!LoRa.begin(freq)) {
        Serial.println(F("Error: LoRa Module Failure"));
        while (1)
            ERROR_OUT(ERROR_NO_LORA);
    }

    LoRa.setSpreadingFactor(SPREADFACTOR);
    LoRa.setSignalBandwidth(CHIRPBW);
    LoRa.setCodingRate4(CODERATE);
    LoRa.setPreambleLength(PREAMBLELEN);
    LoRa.setSyncWord(SYNCWORD);
    LoRa.setTxPower(LORA_POWER, PA_OUTPUT_PA_BOOST_PIN);
    LoRa.setGain(RECEIVER_GAINMODE);

    timeStamp();
    if (USING_CRC) {
        LoRa.enableCrc();
        Serial.print(F("CRC Enabled: "));
    }
    else {
        LoRa.disableCrc();
        Serial.print(F("CRC Disabled: "));
    }
    Serial.println(F("LoRa Module Operational!"));

    // POWER MANAGER INITIALIZATION

    PMIC.begin();
    // here
    PMIC.end();
    timeStamp();
    Serial.println(F("Power Manager IC Initialized!"));

    // TEMPERATURE SENSOR (IF AVAILABLE)
    tSensor = new DHT(PIN_DHT, DHT11);
    tSensor->begin();
    delay(2000); // required after setup
    if (!isnan(tSensor->readTemperature(false))) {
        timeStamp();
        Serial.println(F("Temperature Sensor Detected"));
    }
    else {
        timeStamp();
        Serial.println(F("No Temperature Sensor"));
    }

    // Calculate & Print FIR
    populateFIR(LPF_FIR);

    // CALIBRATE GRAVITATIONAL BIAS
    DC_offset = getDCOffset(adxl, CALIBRATION_TIME_SLICE);
    TOLC      = millis();

    // COLLECT SOME STRUCT PARAMETERS

    mnd.ID         = MY_IDENTIFIER;
    mnd.packetnum  = 0;
    mnd.all_states = 0x0F80E402;

    // ISR ATTACHMENT
    motionDetected = false;
    LowPower.attachInterruptWakeup(
        digitalPinToInterrupt(PIN_INTERRUPT), wakeuphandler, RISING); // wake up on pin 7 rising

    // acknowledgement attach
    LoRa.onReceive([](int sz) -> void { ackFlag = sz; });

    // FINALIZE
    indicateOff();
    timeStamp();
    Serial.println(F("Node Setup Complete!"));

    if (!DEBUG) {
        Serial.println(F("Shutting down external serial interface."));
        if (Serial)
            Serial.end();
        USBDevice.detach();
    }
}

void loop_mkr1310() {
    // execution resumes from sleep here
    rtc.disableAlarm();

    attackCounter = 0;
    severityLevel = 0;
    // Enter into this loop if the node is in motion and prepare for detection logic
    if (motionDetected) {

        timeStamp();
        Serial.println(F("Awake due to motion"));

        // collection logic variables
        int x, y, z;

        motionDetected = false;
        // Data Collection mode
        detachInterrupt(digitalPinToInterrupt(PIN_INTERRUPT));
        need_reattach = true;

        //***************DATA COLLECTION LOOP*****************//

        while (++attackCounter <= MAXIMUM_SCANS) {
            // collect data:
            int i = 0;
            adxlMode(adxl, ADXL_COLLECTION); // change to collection mode
            adxl->getInterruptSource();
            // store data for ADXL_SAMPLE_LENGTH samples
            while (i < ADXL_SAMPLE_LENGTH) {

                adxl->readAccel(&x, &y, &z);
                Z_Power_Samples[i] = sq((z - DC_offset) * GRAVITY / (double) ADXL_LSB_PER_G_Z);

                TOLS = millis();

                while (!digitalRead(PIN_INTERRUPT) && (TSLS() < ADXL_SAMPLE_TIMEOUT))
                    // wait for the pin to go high and take sample
                    ;

                i++;
            }

            //****** Detection Logic ******//

            // Check for inactivity and stop reading if there is data below threshold
            if (inactivityInDataEnd(Z_Power_Samples, ADXL_TIME_REST, adxl) && attackCounter <= (MAXIMUM_SCANS - 1)) {
                timeStamp();
                Serial.println(F("No need to scan again"));
                break;
            }
            else if (attackCounter != MAXIMUM_SCANS) {
                timeStamp();
                Serial.println(F("Still some movement, continuing scan"));
            }
        }
        //***************END DATA COLLECTION LOOP*****************//

        timeStamp();
        severityLevel = getFilteredSeverity(severityLevel, Z_Power_Samples, LPF_FIR);
        Serial.print("Peak severity of ");
        Serial.print(severityLevel);
        Serial.println(". Exit.");
    }

    // Immediately reprepare to detect motion
    adxlMode(adxl, ADXL_MOTION);

    if (need_reattach) {
        LowPower.attachInterruptWakeup(digitalPinToInterrupt(PIN_INTERRUPT), wakeuphandler, RISING);
        need_reattach = false;
    }
    adxl->getInterruptSource();
    motionDetected = false; // attaching an interrupt may cause it to run. unset it here.

    errorOn();
    // get basic status indicators
    mnd.packetnum += 1;
    mnd.upTime = millis();
    mnd.epoch  = rtc.getEpoch();

    // compact-byte types:
    setSeverity(mnd, severityLevel);
    severityLevel = 0; // and viola, reset

    setTSLC(mnd, TSLC() / 60000UL); // convert ms to minutes;

    setTemperature(mnd, (int) tSensor->readTemperature(false));

    AccelData dum;
    adxl->getInterruptSource();
    adxl->readAccel(&dum.x, &dum.y, &dum.z);
    setIMUBit(mnd, ((abs(dum.x) + abs(dum.y) + abs(dum.z)) > 0));
    if (!getIMUBit) {
        TOLC = 0;
        fullResetADXL(adxl); // reset on failure
    }

    int bat_raw = analogRead(PIN_BATADC);
    // convert ADC reading to an estimated percentage and store
    setBatt(mnd,
            100 * (((ADC_VREF * (bat_raw) / ((0b1 << ADC_BITS) - 1)) * (R_top + R_bot) / R_bot) - VBAT_ZERO) /
                (VBAT_HUNDRED - VBAT_ZERO));

    setConnections(mnd, 1);

    // WAIT UNTIL TIME TO SEND IS UPON US

    if (rtc.getEpoch() >= next_wake_epoch + 1) { // triggers if had a long collection
        // find out when to set my alarm for
        next_wake_epoch = rtc.getEpoch() + (SLEEP_TIME_MS / 1000);
        while (next_wake_epoch % (SLEEP_TIME_MS / 1000) != 0)
            next_wake_epoch--;
        next_wake_epoch += MY_IDENTIFIER;
    }
    next_send_epoch = next_wake_epoch + 1; // generally actually the "last" wake epoch

    timeStamp();
    Serial.println(F("Awake before epoch. Waiting for my turn..."));
    while (rtc.getEpoch() < next_send_epoch) {
        yield(); // ? chilling?
    }

    // show what we are sending
    timeStamp();
    Serial.print(F("Sending Data: 0x"));
    for (int k = 0; k < sizeof(MND_Compact); ++k) {
        uint8_t b = ((uint8_t *) &mnd)[k];

        if (b <= 0xF)
            Serial.print("0");
        Serial.print(b, 16);
    }
    Serial.println();

    // send status indicators
    LoRa.beginPacket();
    LoRa.write((uint8_t *) &mnd, sizeof(MND_Compact)); // Write the data to the packet
    LoRa.endPacket(false);                             // false to block while sending

    timeStamp();
    Serial.println(F("Sent!"));
    errorOff();

    // get acknowledgement from central node
    // todo: don't do this every time, propagate through network
    LoRa.receive();

    TOLA = millis();
    while (!ackFlag && (TSLA() < ACK_TIMEOUT))
        yield();

    if (ackFlag > 0) {

        int byteIndexer = 0;
        while (LoRa.available() && byteIndexer < sizeof(MND_Ack))
            ((uint8_t *) &mnd_response)[byteIndexer++] = (uint8_t) LoRa.read();

        timeStamp();
        Serial.print("Got: ");
        Serial.print(mnd_response.universal_epoch);
        Serial.print("  ");
        Serial.print(mnd_response.central_millis);
        Serial.print("  ");
        Serial.println(mnd_response.weak_signal_please_increase);

        if (getNeedRTC(mnd)) {
            rtc.setEpoch(mnd_response.universal_epoch);
            setNeedRTC(mnd, false);
            timeStamp();
            Serial.println(F("Clocks Aligned"));
        }

        central_milliseconds = mnd_response.central_millis;
        timeStamp();
        Serial.println(F("Subclock MS Value Set"));
    }
    else {
        timeStamp();
        Serial.println(F("No response from central"));
    }
    // reset receiver flag and sleep transmitter
    ackFlag = 0;
    LoRa.sleep();

    if (!motionDetected) { // no motion during sending phase, safe to sleep or perform calibration

        // check if we should recompute the DC offset
        if (TSLC() > ADXL_CALIBRATION_INTERVAL) {
            DC_offset     = getDCOffset(adxl, CALIBRATION_TIME_SLICE);
            need_reattach = true; // after the offset you need this
            TOLC          = millis();
        }

        // find out when to set my alarm for
        next_wake_epoch = rtc.getEpoch() + (SLEEP_TIME_MS / 1000);
        while (next_wake_epoch % (SLEEP_TIME_MS / 1000) != 0)
            next_wake_epoch--;

        next_wake_epoch += MY_IDENTIFIER;

        timeStamp();
        Serial.print("Sleeping until ");
        Serial.println(next_wake_epoch);

        // real sleeping:
        // LowPower.deepSleep(next_wake_epoch - rtc.getEpoch());
        // simulated sleeping:
        while (!digitalRead(PIN_INTERRUPT) && rtc.getEpoch() < next_wake_epoch)
            ;
    }
}

#endif
