#include "3.h"
#include "unity.h"

#include "dfm_mkr1310.h"
#include "dfm_utils.h"

#include <Arduino.h>
#include <RTCZero.h>
#include <SPI.h>
#include <LoRa.h>
#include <Arduino_PMIC.h>
#include <ArduinoLowPower.h>
#include <ArduinoECCX08.h>
#include <SparkFun_ADXL345.h>  

MonitoringNodeData mnd_test;

ADXL345 adxl = ADXL345(10);

void setUp(void) {
    // set stuff up here
    pinMode(PIN_STATUSLED, OUTPUT);
    indicateOff();
    
    delay(2000);
    runUnityTests();

    //Enable Arduino interrupts
    interrupts();

    //ADXL345 setup

    //Turn it on.
    adxl.powerOn();

    adxl.setRangeSetting(16);           // Give the range settings
                                        // Accepted values are 2g, 4g, 8g or 16g
                                        // Higher Values = Wider Measurement Range
                                        // Lower Values = Greater Sensitivity

    adxl.setSpiBit(0);                  // Configure the device to be in 4 wire SPI mode when set to '0' or 3 wire SPI mode when set to 1
                                        // Default: Set to 1
                                        // SPI pins on the ATMega328: 11, 12 and 13 as reference in SPI Library 
   
    adxl.setActivityXYZ(1, 1, 1);       // Set to activate movement detection in the axes "adxl.setActivityXYZ(X, Y, Z);" (1 == ON, 0 == OFF)
    adxl.setActivityThreshold(75);      // 62.5mg per increment   // Set activity   // Inactivity thresholds (0-255)
 
    adxl.setInactivityXYZ(1, 1, 1);     // Set to detect inactivity in all the axes "adxl.setInactivityXYZ(X, Y, Z);" (1 == ON, 0 == OFF)
    adxl.setInactivityThreshold(75);    // 62.5mg per increment   // Set inactivity // Inactivity thresholds (0-255)
    adxl.setTimeInactivity(1);          // How many seconds of no activity is inactive?

    adxl.setTapDetectionOnXYZ(1, 1, 1); // Detect taps in the directions turned ON "adxl.setTapDetectionOnX(X, Y, Z);" (1 == ON, 0 == OFF)
 
    // Set values for what is considered a TAP and what is a DOUBLE TAP (0-255)
    adxl.setTapThreshold(50);           // 62.5 mg per increment
    adxl.setTapDuration(15);            // 625 μs per increment

    // Set values for what is considered a DOUBLE TAP (0-255)
    adxl.setDoubleTapLatency(80);       // 1.25 ms per increment
    adxl.setDoubleTapWindow(200);       // 1.25 ms per increment
    
    // Set values for what is considered FREE FALL (0-255)
    adxl.setFreeFallThreshold(7);       // (5 - 9) recommended - 62.5mg per increment
    adxl.setFreeFallDuration(30);       // (20 - 70) recommended - 5ms per increment

    // Turn on Interrupts for each mode (1 == ON, 0 == OFF)
    adxl.InactivityINT(1);
    adxl.ActivityINT(1);
    adxl.FreeFallINT(1);
    adxl.doubleTapINT(1);
    adxl.singleTapINT(1);

}

void tearDown(void) {
    // clean stuff up here

    //disable Arduino interrupts
    noInterrupts();

    //disable ADXL interrupts
    adxl.InactivityINT(0);
    adxl.ActivityINT(0);
    adxl.FreeFallINT(0);
    adxl.doubleTapINT(0);
    adxl.singleTapINT(0);
}

//
//Interrupt Service Routine.
void ISR_test(){
    printf("Interrupt!\n");

    byte interruptSource = adxl.getInterruptSource();

    if(adxl.triggered(interruptSource, ADXL345_ACTIVITY)) {
        printf("Activity detected!\n");

        //and also do other stuff when we detect motion
    }


}

void test_function(void) {
    indicateOn();
    TEST_ASSERT_TRUE((LoRa.begin(LoRaChannelsUS[1])));
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

    LoRa.sleep();
    indicateOff();
    delay(500);
    LowPower.deepSleep(500);

    for (int tx = 2; tx <= 18; ++tx) {
        mnd_test.upTime = millis();
        LoRa.setTxPower((tx == 18) ? 20 : tx, PA_OUTPUT_PA_BOOST_PIN);
        LoRa.beginPacket();
        LoRa.write((uint8_t *) &mnd_test, sizeof(MonitoringNodeData));
        indicateOn();
        LoRa.endPacket(false); // async blocked
        indicateOff();
        delay(500);
    }
}

int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_function);
    return UNITY_END();
}

//Accelerometer readings and interrupt. 
void loop() {
    int x, y, z;

    adxl.readAccel(&x, &y, &z);

    //Prints readings
    printf("%d, %d, %d\n", x, y, z);

    ISR_test();
}