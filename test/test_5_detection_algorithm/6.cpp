#include "5.h"
#include "unity.h"

#include "dfm_mkr1310.h"
#include "dfm_utils.h"

#include <Arduino.h>
#include <ArduinoECCX08.h>
#include <ArduinoLowPower.h>
#include <Arduino_PMIC.h>
#include <LoRa.h>
#include <RTCZero.h>
#include <SPI.h>
#include <SparkFun_ADXL345.h>

MonitoringNodeData mnd_test;

ADXL345 adxl = ADXL345(1);

// Interrupt Service Routine.
void ISR_test() {
    Serial.print("Interrupt!\n");

    byte interruptSource = adxl.getInterruptSource();

    if (adxl.triggered(interruptSource, ADXL345_ACTIVITY)) {
        Serial.print("Activity detected!\n");

        // And also do other stuff when we detect motion:
        // Set the interrupt flag to true
        // Run the detection algorithm and fill out the array
        interrupted = true;
        for (size_t i = 0; i < SIZE; i++) {
            detection();
        }
    }
}

void test_function(void) {

    // ADXL345 setup
    // Turn it on.
    adxl.powerOn();

    adxl.setRangeSetting(4); // 4g

    adxl.setSpiBit(0); // Configure the device to be in 4 wire SPI mode when set to '0' or 3 wire SPI mode when set to 1
                       // Default: Set to 1
                       // SPI pins on the ATMega328: 11, 12 and 13 as reference in SPI Library

    adxl.setActivityXYZ(1, 1, 1);  // Set to activate movement detection in the axes (1 == ON, 0 == OFF)
    adxl.setActivityThreshold(75); // 62.5mg per increment   // Set activity   // Inactivity thresholds (0-255)

    adxl.setInactivityXYZ(1, 1, 1);  // Set to detect inactivity in all the axes (1 == ON, 0 == OFF)
    adxl.setInactivityThreshold(75); // 62.5mg per increment   // Set inactivity // Inactivity thresholds (0-255)
    adxl.setTimeInactivity(1);       // How many seconds of no activity is inactive?

    // Turn on Interrupts for each mode (1 == ON, 0 == OFF)
    adxl.InactivityINT(1);
    adxl.ActivityINT(1);

    // Enable Arduino interrupts
    interrupts();
}

// Collect and record the data from the accelerometer
void detection() {
    int x, y, z;

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

// added code to be sorted later to ensure no confliction---------------------------------------------------------
#define ARRAY_MAX     100
#define DEFAULT_TRESH 75

    // variable delcaration
    int accelX, accelY, accelZ;
    int thresholdX = DEFAULT_TRESH;
    int thresholdY = DEFAULT_TRESH;
    int thresholdZ = DEFAULT_TRESH;
    double energyX, energyY, energyZ;
    int abvThreshX = 0;
    int abvThreshY = 0;
    int abvThreshZ = 0;
    int blwThreshX = 0;
    int blwThreshY = 0;
    int blwThreshZ = 0;

    double FenceTouchedX[ARRAY_MAX]; // again how big do we expected the data to be
    double FenceTouchedY[ARRAY_MAX]; // should I also use accel_reading to keep it simple?
    double FenceTouchedZ[ARRAY_MAX];
    // did we want an array for not touched fence -- yes

    for (unsigned int index = 0; a < sizeof(x_accelerometer);
         index++) { // sizeof(x_accelerometer) or accel_reading --- which would be better
        accelX  = x_accelerometer[index];
        accelY  = Y_accelerometer[index];
        accelZ  = Z_accelerometer[index];
        energyX = pow(accelX, 2.0); // forgot times (t)time
        energyY = pow(accelY, 2.0); // should the time be based on the internal clock?
        energyZ = pow(accelZ, 2.0); // we can I look for finding code to interact with the microprocessor
        if (accelX > thresholdX) {
            FenceTouchedX[index] = accelX; // do we want the data or the energy
            abvThreshX++;
        }
        else {
            blwThreshX++;
        }
        if (accelY > thresholdY) {
            FenceTouchedY[index] = accelY;
            abvThreshY++;
        }
        else {
            blwThreshY++;
        }
        if (accelZ > thresholdZ) {
            FenceTouchedZ[index] = accelZ;
            abvThreshZ++;
        }
        else {
            blwThreshZ++;
        }
    }
}

// do not modify the functions below!

int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_function);
    return UNITY_END();
}
void setUp(void) {}
void setup() {
    delay(2000);
    runUnityTests();
}
void loop() {}
void tearDown(void) {}
