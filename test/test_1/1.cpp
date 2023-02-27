#include "1.h"
#include "unity.h"

#include <Arduino.h>
#include <SPI.h>
#include <SparkFun_ADXL345.h>
#include <Wire.h>

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_function(void) {
    ADXL345 adxl = ADXL345(1); // CS pin is pin 1

    adxl.powerOn();
    adxl.setRangeSetting(2); // gs

    for (;;) {

        int x, y, z;
        unsigned long toli = 0;
        while ((millis() - toli) <= ceil(1000 / 104))
            ;
        adxl.readAccel(&x, &y, &z);
        toli = millis();
        if (x)
        Serial.print(x);
        Serial.print(",\t");
        Serial.print(y);
        Serial.print(",\t");
        Serial.print(z);
        Serial.println();
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
