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
        int arrayx[100]; // entries for x; 
        int arrayy[100]; // entries for y;
        int arrayz[100];// entries for z;
        int i = 0;
        int x, y, z;

    for (;;) {
       
        unsigned long toli = 0;
        while ((millis() - toli) <= ceil(1000 / 104))
            ;
        
        adxl.readAccel(&x, &y, &z);
        arrayx[i % 100] = x;
        arrayy[i % 100] = y;
        arrayz[i & 100] = z;
        i++;
        toli = millis();
       
        Serial.print(arrayx[i]);
        Serial.print(",\t");
        Serial.print(arrayy[i]);
        Serial.print(",\t");
        Serial.print(arrayz[i]);
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
