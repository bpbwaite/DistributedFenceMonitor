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
    ADXL345 adxl = ADXL345();

    adxl.powerOn();
    adxl.setRangeSetting(4); // gs

    for (;;) {

        int x, y, z;
        unsigned long toli = 0;
        double cx = 0, cy = 0, cz = 0;
        for (int a = 0; a < 32; ++a) {
            while ((millis() - toli) <= ceil(1000 / 104))
                ;
            adxl.readAccel(&x, &y, &z);
            toli = millis();
            cx += x;
            cy += y;
            cz += z;
        }
        cx /= 32.0;
        cy /= 32.0;
        cz /= 32.0;

        Serial.print((cx - 4) * (1.0 / -130.0));
        Serial.print(",\t");
        Serial.print((cy - 3) * (-1.0 / 133.0));
        Serial.print(",\t");
        Serial.print((cz + 14) * (1.0 / 128.0));
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
