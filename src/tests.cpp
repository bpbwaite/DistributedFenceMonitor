
#include "tests.h"

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#if defined(ARDUINO_SAMD_MKRWAN1310)
#include "dfm_mkr1310.h"
#include "dfm_utils.h"

#include <ArduinoECCX08.h>
#include <ArduinoLowPower.h>
#include <LoRa.h>
#include <RTCZero.h>

#include <SparkFun_ADXL345.h>

#if !defined(CENTRAL_NODE)
#include <Arduino_PMIC.h>
#endif
// global variables accessed in ISRs:
volatile bool motionDetected = false;

// global variables and objects
AccelData accelData;

// Interrupt Service Routines:
void isr() {

    // printf("Interrupt!\n");
    motionDetected = true;

    // byte interruptSource = adxl.getInterruptSource();

    //  if (adxl.triggered(interruptSource, ADXL345_ACTIVITY)) {
    //  printf("Activity detected!\n");//serial print
    // and also do other stuff when we detect motion

    // }
}

void test_accel(void) {
    static int arrayx[10]; // entries for x;
    static int arrayy[10]; // entries for y;
    static int arrayz[10]; // entries for z;

    delay(500);

    ADXL345 adxl = ADXL345(1); // CS pin is pin 1

    if (!Serial)
        Serial.begin(115200);

    adxl.powerOn();
    adxl.setSpiBit(0);
    adxl.setRangeSetting(2); // gs
    // adxl.set_bw(ADXL345_BW_100);
    Serial.println(adxl.get_bw_code());
    adxl.setInterrupt(ADXL345_DATA_READY, true);
    adxl.setInterruptMapping(ADXL345_DATA_READY, ADXL345_INT2_PIN);

    delay(3000);

    int i = 0;
    int x, y, z;

    for (;;) {

        unsigned long toli = 0;
        while ((millis() - toli) <= ceil(1000 / 104))
            ;

        adxl.readAccel(&x, &y, &z);
        toli = millis();

        arrayx[i % 10] = x;
        arrayy[i % 10] = y;
        arrayz[i % 10] = z;

        Serial.print(x);
        Serial.print(",\t");
        Serial.print(y);
        Serial.print(",\t");
        Serial.print(z);
        Serial.println();

        i++;
    }
}

void test_stream(void) {

    ADXL345 adxl = ADXL345(1); // CS pin is pin 1

    if (!Serial)
        Serial.begin(115200);

    adxl.powerOn();
    adxl.setSpiBit(0);
    adxl.setRangeSetting(2); // gs
    // adxl.set_bw(ADXL345_BW_100);
    // Serial.println(adxl.get_bw_code());
    adxl.setInterrupt(ADXL345_DATA_READY, true);
    adxl.setInterruptMapping(ADXL345_DATA_READY, ADXL345_INT2_PIN);

    //  delay(3000);

    accelData.x        = 0;
    accelData.y        = 0;
    accelData.z        = 0;
    unsigned long toli = 0;

    for (;;) {

        while ((millis() - toli) <= ceil(1000 / 104))
            ;

        adxl.readAccel(&accelData.x, &accelData.y, &accelData.z);
        toli = millis();

        Serial.write((unsigned char) '#');
        adtomatlab(Serial, accelData);
        Serial.println();
    }
}

void adtomatlab(Serial_ &s, const AccelData d) {
    for (int nb = 0; nb < sizeof(AccelData); ++nb)
        s.write((unsigned char) ((uint8_t *) &d)[nb]);
}

void test_power(void) {
    pinMode(PIN_STATUSLED, OUTPUT);
    indicateOff();
    MonitoringNodeData mnd_test;

    indicateOn();
    LoRa.begin(LoRaChannelsUS[1]);
    LoRa.setSpreadingFactor(SPREADFACTOR);
    LoRa.setSignalBandwidth(CHIRPBW);
    LoRa.setSyncWord(SYNCWORD);
    LoRa.setPreambleLength(PREAMBLELEN);

    LoRa.enableCrc(); // unconditionally

    mnd_test.ID             = 0xFE;
    mnd_test.freq           = LoRaChannelsUS[1];
    mnd_test.SyncWord       = SYNCWORD;
    mnd_test.packetnum      = 0;
    mnd_test.connectedNodes = 0;
    mnd_test.timeOnAir      = 0;
    mnd_test.status         = 0b10101010;

    while (true) {

        LoRa.sleep();
        indicateOff();
        delay(500);
        LowPower.deepSleep(1000);

        for (int tx = 2; tx <= 18; ++tx) {
            mnd_test.upTime = millis();
            LoRa.setTxPower((tx == 18) ? 20 : tx, PA_OUTPUT_PA_BOOST_PIN);
            LoRa.beginPacket();
            LoRa.write((uint8_t *) &mnd_test, sizeof(MonitoringNodeData));
            indicateOn();
            LoRa.endPacket(false); // async blocked
            indicateOff();
            delay(100);
        }
    }
}
void test_isr2(void) {
    ADXL345 adxl = ADXL345(1);

    // LowPower.attachInterruptWakeup(digitalPinToInterrupt(2), NULL, RISING); // attach interrupt to pin 2
    attachInterrupt(digitalPinToInterrupt(2), isr, CHANGE); // attach interrupt to pin 2

    // ADXL345 setup
    // Turn it on.
    adxl.powerOn();

    adxl.setRangeSetting(4); // 4g

    adxl.setSpiBit(0); // Configure the device to be in 4 wire SPI mode when set to '0' or 3 wire SPI mode when set to 1
                       // Default: Set to 1
                       // SPI pins on the ATMega328: 11, 12 and 13 as reference in SPI Library

    adxl.setActivityXYZ(1, 1, 1);  // Set to activate movement detection in the axes (1 == ON, 0 == OFF)
    adxl.setActivityThreshold(75); // 62.5mg per increment   // Set activity   // Inactivity thresholds (0-255)
    // Turn on Interrupts for each mode (1 == ON, 0 == OFF)
    adxl.ActivityINT(1);

    while (1) {
        while (!motionDetected) {
        }
        Serial.println("Motion detected!");
        motionDetected = false;
    }
}

void test_russey_mobile(void) {
#define PIN_INCREMENT_PANEL A6
#define PIN_SAMPLE_PANEL    A5
#define DEBOUNCERINO        200
#define SAMPLES_PER_PUSH    5

    RTCZero rtc;
    MonitoringNodeData mnd;
    rtc.begin();
    rtc.setEpoch(COMPILE_TIME);
    rtc.disableAlarm();

    Serial.begin(115200);

    pinMode(PIN_LORAMODE, INPUT_PULLUP);
    pinMode(PIN_STATUSLED, OUTPUT);
    pinMode(PIN_ERRORLED, OUTPUT);
    pinMode(PIN_INCREMENT_PANEL, INPUT_PULLUP);
    pinMode(PIN_SAMPLE_PANEL, INPUT_PULLUP);

    long freq = LoRaChannelsUS[63];

    indicateOn();
    if (!LoRa.begin(freq)) {
        Serial.println("LORA BAD");
        while (1)
            ;
    }
    Serial.println("LORA ONLINE");

    LoRa.setSpreadingFactor(SPREADFACTOR);
    LoRa.setSignalBandwidth(CHIRPBW);
    LoRa.setSyncWord(SYNCWORD);
    LoRa.setPreambleLength(PREAMBLELEN);
    // default 17 is very powerful, trips OCP sometimes. minimum 2
    LoRa.setTxPower(15, PA_OUTPUT_PA_BOOST_PIN);
    LoRa.enableCrc();
    mnd.ID   = 0x99;
    mnd.freq = freq;

    mnd.SyncWord       = SYNCWORD;
    mnd.packetnum      = 0;
    mnd.connectedNodes = 0;
    mnd.timeOnAir      = 0;

    mnd.status = 0b00000000;

    for (;;) {

        while (!digitalRead(PIN_SAMPLE_PANEL))
            ;
        while (digitalRead(PIN_SAMPLE_PANEL)) {
            if (!digitalRead(PIN_INCREMENT_PANEL)) {
                mnd.status += 1;
                Serial.print(F("Incremented Panel, i = "));
                Serial.println(mnd.status);
                delay(DEBOUNCERINO);
                while (!digitalRead(PIN_INCREMENT_PANEL))
                    ;
            }
        }
        for (int k = 0; k < SAMPLES_PER_PUSH; ++k) {
            delay(100); // don't overwhelm silly little Matlab

            mnd.upTime = millis();
            mnd.epoch  = rtc.getEpoch();
            mnd.bat    = analogRead(PIN_BATADC);

            indicateOn();
            mnd.packetnum += 1;
            LoRa.beginPacket();
            LoRa.write((uint8_t *) &mnd, sizeof(MonitoringNodeData));
            LoRa.endPacket(false);
            mnd.timeOnAir += getTOA(sizeof(MonitoringNodeData));
            indicateOff();
        }
        Serial.println("Sent Sample :3");
        delay(DEBOUNCERINO);
    }
}
void test_russey_station(void) {
    MonitoringNodeData mndBuf;
    long freq = LoRaChannelsUS[63];
    Serial.begin(SERIALBAUD);
    while (!Serial && millis() < SERIALTIMEOUT)
        yield();

    pinMode(PIN_STATUSLED, OUTPUT);
    pinMode(LORA_IRQ, INPUT);

    indicateOn();
    if (!LoRa.begin(freq)) {
        while (1)
            ;
    }
    LoRa.setGain(RECEIVER_GAINMODE);
    LoRa.setSpreadingFactor(SPREADFACTOR);
    LoRa.setSignalBandwidth(CHIRPBW);
    LoRa.setSyncWord(SYNCWORD);
    LoRa.setPreambleLength(PREAMBLELEN);
    LoRa.enableCrc();

    for (;;) {
        indicateOff();

        while (!LoRa.parsePacket())
            ;

        indicateOn();

        Serial.print('#');

        int byteIndexer = 0;
        while (LoRa.available() && byteIndexer < sizeof(MonitoringNodeData))
            ((uint8_t *) &mndBuf)[byteIndexer++] = (uint8_t) LoRa.read();

        ReceiverExtras r = {
            LoRa.packetRssi(),
            LoRa.packetSnr(),
            CHIRPBW,
            SPREADFACTOR,
        };
        frssitomatlab(Serial, mndBuf, r);
        Serial.println();
    }
}
void frssitomatlab(Serial_ &s, const MonitoringNodeData d, const ReceiverExtras r) {

    static TestPing p;

    p.panelNum = d.status;
    p.rssi     = r.rssi;
    p.snr      = r.snr;

    // Serial.print("status");
    // Serial.println(d.status);
    // Serial.print("rssi");
    // Serial.println(r.rssi);

    for (int nb = 0; nb < sizeof(TestPing); ++nb)
        s.write((unsigned char) ((uint8_t *) &p)[nb]);
}
#endif

#if defined(ARDUINO_NANO_RP2040_CONNECT)
#include <ArduinoBLE.h>

void test_nanoble(void) {
    int buttonPin = 2;
    boolean ledSwitch;
    BLEService LEDService("19B10000-E8F2-537E-4F6C-D104768A1214"); // BLE LED Service
    // BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central
    BLEByteCharacteristic LEDCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify | BLEWrite);

    Serial.begin(9600);
    pinMode(buttonPin, INPUT_PULLUP);
    // begin initialization
    if (!BLE.begin()) {
        Serial.println("starting Bluetooth® Low Energy failed!");
    }
    // set advertised local name and service UUID:
    BLE.setLocalName("Button Device");
    BLE.setAdvertisedService(LEDService);
    // add the characteristic to the service
    LEDService.addCharacteristic(LEDCharacteristic);
    // add service
    BLE.addService(LEDService);
    // start advertising
    BLE.advertise();
    Serial.println("BLE LED Peripheral, waiting for connections....");
    while (1) {
        // listen for BLE peripherals to connect:
        BLEDevice central = BLE.central();
        // if a central is connected to peripheral:
        if (central) {
            Serial.print("Connected to central: ");
            // print the central's MAC address:
            Serial.println(central.address());
            // while the central is still connected to peripheral:
            while (central.connected()) {

                int buttonState = digitalRead(buttonPin);

                if (buttonState == LOW) {
                    ledSwitch = !ledSwitch;
                    delay(500);

                    if (ledSwitch) {
                        Serial.println("ON");
                        LEDCharacteristic.writeValue((byte) 0x01);
                    }
                    else {
                        LEDCharacteristic.writeValue((byte) 0x00);
                        Serial.println("OFF");
                    }
                }
            }
            // when the central disconnects, print it out:
            Serial.print(F("Disconnected from central: "));
            Serial.println(central.address());
        }
    }
}

#endif
