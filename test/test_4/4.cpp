#include "4.h"
#include "unity.h"

#include <Arduino.h>
#include <ArduinoBLE.h>
#include <SPI.h>

int buttonPin = 2;
boolean ledSwitch;
BLEService LEDService("19B10000-E8F2-537E-4F6C-D104768A1214"); // BLE LED Service
// BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic LEDCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify | BLEWrite);

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_function(void) {
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

int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_function);
    return UNITY_END();
}
void setup() {
    delay(2000);

    runUnityTests();
}
void loop() {}
