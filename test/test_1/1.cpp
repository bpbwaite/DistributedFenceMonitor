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
        int arrayx[100];
        int arrayy[100];
        int arrayz[100];
        int i =0;

        int x, y, z;
        unsigned long toli = 0;
        while ((millis() - toli) <= ceil(1000 / 104))
            ;
        adxl.readAccel(&x, &y, &z);
        arrayx[i] = x;
        arrayy[i] = y;
        arrayz[i] = z;
        i++;
        toli = millis();
       
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
