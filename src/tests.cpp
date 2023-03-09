
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
    motionDetected = true;
}

void test_wake(void) {

    MonitoringNodeData mnd_test;
    ADXL345 adxl = ADXL345(1);
    int x, y, z;

    pinMode(7, INPUT);
    pinMode(PIN_STATUSLED, OUTPUT);
    indicateOff();

    Serial.begin(SERIALBAUD);
    while (!Serial && millis() < SERIALTIMEOUT)
        yield();
    Serial.println(F("Notice: Serial Interface Connected!"));

    LoRa.begin(LoRaChannelsUS[1]);
    LoRa.setSpreadingFactor(SPREADFACTOR);
    LoRa.setSignalBandwidth(CHIRPBW);
    LoRa.setSyncWord(SYNCWORD);
    LoRa.setPreambleLength(PREAMBLELEN);
    LoRa.enableCrc();

    LoRa.setTxPower(15, PA_OUTPUT_PA_BOOST_PIN);

    mnd_test.ID     = 0x99;
    mnd_test.status = 0b00000000;
    mnd_test.upTime = millis();
    mnd_test.bat    = analogRead(PIN_BATADC);

    // ADXL345 setup
    adxl.powerOn();
    adxl.setSpiBit(0); // 4-Wire SPI
    adxl.setRangeSetting(2);

    adxl.setFullResBit(1);
    adxl.set_bw(ADXL345_BW_50);
    // adxl.setRate(); // i2c method?

    adxl.setInterruptLevelBit(0); // means the pin RISES on interrupt

    adxl.setInterrupt(ADXL345_DATA_READY, true);
    adxl.setInterruptMapping(ADXL345_DATA_READY, ADXL345_INT1_PIN);

    adxl.setActivityAc(1);         // AC coupled activitiy
    adxl.setActivityXYZ(1, 1, 1);  // Set to activate movement detection in the axes (1 == ON, 0 == OFF)
    adxl.setActivityThreshold(25); // 62.5mg per increment   // Set activity   // Inactivity thresholds (0-255)
    adxl.ActivityINT(1);
    adxl.setInterruptMapping(ADXL345_ACTIVITY, ADXL345_INT2_PIN);
    // ADXL345_FIFO_STATUS; // can we change?

    adxl.InactivityINT(0);
    adxl.ActivityINT(1);
    adxl.FreeFallINT(0);
    adxl.doubleTapINT(0);
    adxl.singleTapINT(0);
    adxl.setInterruptMapping(ADXL345_OVERRUNY, ADXL345_INT1_PIN);
    adxl.setInterruptMapping(ADXL345_WATERMARK, ADXL345_INT1_PIN);

    Serial.println("ADXL REGISTER STATUS AFTER SETUP:");
    adxl.printAllRegister();

    noInterrupts();

    LowPower.attachInterruptWakeup(
        7, []() -> void {}, RISING);

    attachInterrupt(7, isr, RISING);

    delay(1000); // needs some time to boot? determine empirically please

    while (digitalRead(7)) {
        adxl.getInterruptSource();
        adxl.readAccel(&x, &y, &z); // clears interrupt if present, clear FIFO
    }

    Serial.println(F("Interrupt Attached!"));
    Serial.end(); //:(

    interrupts();
    AccelData dummy;

    for (;;) {
        LoRa.sleep();
        LowPower.deepSleep();
        // execution resumes after waking up:

        indicateOn();
        if (motionDetected) {

            motionDetected = false;

            byte whichInterrupt = adxl.getInterruptSource();

            adxl.readAccel(&dummy.x, &dummy.y, &dummy.z);

            LoRa.beginPacket();
            LoRa.write((uint8_t *) &mnd_test, sizeof(MonitoringNodeData));
            LoRa.endPacket(false);
            indicateOff();

            // dont resend too fast
            while (digitalRead(7)) {
                adxl.getInterruptSource();
                delay(10);
            }
        }
    }
}
void test_detection(void) {

#define SIZE           1000 // The number of accelerometer readings to hold
#define DEFAULT_THRESH 75
    int x_accelerometer[SIZE]; // Data from the accelerometer's x-axis
    int y_accelerometer[SIZE]; // Data from the accelerometer's y-axis
    int z_accelerometer[SIZE]; // Data from the accelerometer's z-axis
    int accel_reading = 0;     // Location within the accelerometer data arrays
    bool interrupted  = false; // Interrupt flag
    int x, y, z;
    // variable delcaration
    int accelX, accelY, accelZ;
    int thresholdX = DEFAULT_THRESH;
    int thresholdY = DEFAULT_THRESH;
    int thresholdZ = DEFAULT_THRESH;
    double energyX, energyY, energyZ;
    int abvThreshX = 0;
    int abvThreshY = 0;
    int abvThreshZ = 0;
    int blwThreshX = 0;
    int blwThreshY = 0;
    int blwThreshZ = 0;

    MonitoringNodeData mnd_test;
    ADXL345 adxl = ADXL345(1);

    Serial.begin(SERIALBAUD);
    while (!Serial && millis() < SERIALTIMEOUT)
        yield();
    Serial.println(F("Notice: Serial Interface Connected!"));

    mnd_test.ID             = 0x99;
    mnd_test.freq           = 915000000;
    mnd_test.SyncWord       = SYNCWORD;
    mnd_test.packetnum      = 0;
    mnd_test.connectedNodes = 0;
    mnd_test.timeOnAir      = 0;
    mnd_test.status         = 0b00000000;
    mnd_test.upTime         = millis();
    // mnd.epoch  = rtc.getEpoch();
    mnd_test.bat = analogRead(PIN_BATADC);

    // ADXL345 setup
    adxl.powerOn();
    adxl.setSpiBit(0); // 4-Wire SPI
    adxl.setRangeSetting(2);

    adxl.setFullResBit(1);
    adxl.set_bw(ADXL345_BW_50);
    // adxl.setRate(); // i2c method?

    adxl.setInterruptLevelBit(0); // means the pin RISES on interrupt
    adxl.setInterrupt(ADXL345_DATA_READY, true);
    adxl.setInterruptMapping(ADXL345_DATA_READY, ADXL345_INT2_PIN);
    adxl.setActivityAc(1);        // AC coupled activitiy
    adxl.setActivityXYZ(1, 1, 1); // Set to activate movement detection in the axes (1 == ON, 0 == OFF)
    adxl.setActivityThreshold(5); // 62.5mg per increment   // Set activity   // Inactivity thresholds (0-255)
    // Turn on Interrupts for each mode (1 == ON, 0 == OFF)
    adxl.ActivityINT(1);
    adxl.setInterruptMapping(ADXL345_ACTIVITY, ADXL345_INT1_PIN);
    // ADXL345_FIFO_STATUS; // can we change?

    pinMode(7, INPUT);
    noInterrupts();
    attachInterrupt(7, isr, RISING);
    delay(1000); // needs some time to boot? determine empirically please
    if (digitalRead(7)) {
        for (int i = 0; i < 33; ++i)
            adxl.readAccel(&x, &y, &z); // clears interrupt if present, clear FIFO
    }

    Serial.println(F("Interrupt Attached!"));
    interrupts();
    AccelData dummy;

    for (;;) {

        while (!motionDetected)
            ; // stall to simulate sleeping
        indicateOn();
        motionDetected = false;

        byte whichInterrupt = adxl.getInterruptSource();
        // adxl.setInterrupt(whichInterrupt, false); // try this, usually reading alone works tho
        // for (int j = 0; j < 100; ++j) {
        //  delay(10);
        adxl.readAccel(&dummy.x, &dummy.y, &dummy.z);
        Serial.write((unsigned char) '#');
        adtomatlab(Serial, dummy);
        Serial.println();
        //}
        indicateOff();
    }

    /*
    // Read the accelerometer and record the data in the arrays.
    adxl.readAccel(&x, &y, &z);
    x_accelerometer[accel_reading] = x;
    y_accelerometer[accel_reading] = y;
    z_accelerometer[accel_reading] = z;

    // Note that accel_reading should never be at or above the maximum value of SIZE.
    // Otherwise, we might run into an out-of-bounds error.
    accel_reading++;
    if (accel_reading >= SIZE) {
        accel_reading = 0;
    }
    */
    /*
        for (unsigned int index = 0; a < sizeof(x_accelerometer);
             index++) { // sizeof(x_accelerometer) or accel_reading --- which would be better
            accelX  = x_accelerometer[index];
            accelY  = y_accelerometer[index];
            accelZ  = z_accelerometer[index];
            energyX = pow(accelX, 2.0); // forgot times (t)time
            energyY = pow(accelY, 2.0); // should the time be based on the internal clock?
            energyZ = pow(accelZ, 2.0); // we can I look for finding code to interact with the microprocessor
        }
    */
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
#define PIN_SAMPLE_INCREMENT A6
#define SAMPLES_PER_PUSH     3

    RTCZero rtc;
    MonitoringNodeData mnd;
    rtc.begin();
    rtc.setEpoch(COMPILE_TIME);
    rtc.disableAlarm();

    Serial.begin(115200);

    pinMode(PIN_LORAMODE, INPUT_PULLUP);
    pinMode(PIN_STATUSLED, OUTPUT);
    pinMode(PIN_ERRORLED, OUTPUT);
    pinMode(PIN_SAMPLE_INCREMENT, INPUT_PULLUP);

    long freq = LoRaChannelsUS[1];

    indicateOn();
    if (!LoRa.begin(freq)) {
        // Serial.println("LORA BAD");
        while (1)
            ;
    }
    // Serial.println("LORA ONLINE");

    LoRa.setSpreadingFactor(SPREADFACTOR);
    LoRa.setSignalBandwidth(CHIRPBW);
    LoRa.setSyncWord(SYNCWORD);
    LoRa.setPreambleLength(PREAMBLELEN);
    LoRa.disableCrc();
    mnd.ID   = 0x99;
    mnd.freq = freq;

    mnd.SyncWord       = SYNCWORD;
    mnd.packetnum      = 0;
    mnd.connectedNodes = 0;
    mnd.timeOnAir      = 0;

    mnd.status = 0b00000000;
    indicateOff();

    for (;;) {

        while (!digitalRead(PIN_SAMPLE_INCREMENT))
            ;
        while (digitalRead(PIN_SAMPLE_INCREMENT)) {
            ;
        }
        indicateOn();
        mnd.packetnum += 1;
        for (int k = 0; k < SAMPLES_PER_PUSH; ++k) {
            for (int tx = 2; tx <= 18; ++tx) {
                tx = (tx >= 18 ? 20 : tx);
                LoRa.setTxPower(tx, PA_OUTPUT_PA_BOOST_PIN);
                mnd.status = tx;

                LoRa.beginPacket();
                LoRa.write((uint8_t *) &mnd, sizeof(MonitoringNodeData));
                LoRa.endPacket(false);
                delay(10);
            }
        }
        indicateOff();
        Serial.println("Sent!");
    }
}
void test_russey_station(void) {
    MonitoringNodeData mndBuf;
    long freq = LoRaChannelsUS[1];
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
    LoRa.disableCrc();

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

    p.panelNum = d.packetnum;
    p.rssi     = r.rssi;
    p.txpow    = d.status;
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
