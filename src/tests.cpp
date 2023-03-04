
#include "tests.h"

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#if defined(ARDUINO_SAMD_MKRWAN1310)
#include "dfm_mkr1310.h"
#include "dfm_utils.h"

#include <ArduinoECCX08.h>
#include <ArduinoLowPower.h>
#include <Arduino_PMIC.h>
#include <LoRa.h>
#include <RTCZero.h>

#include <SparkFun_ADXL345.h>

// global variables accessed in ISRs:
volatile bool motionDetected = false;

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

        Serial.print(arrayx[i % 10]);
        Serial.print(",\t");
        Serial.print(arrayy[i % 10]);
        Serial.print(",\t");
        Serial.print(arrayz[i % 10]);
        Serial.println();

        i++;
    }
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
