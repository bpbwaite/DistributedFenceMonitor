#include "testsrun.h"
// #include "unity.h"

#include <Arduino.h>

void setup() {
    delay(2000);
    // UNITY_BEGIN();

    // uncomment tests to run:
    test_accel();
    // test_power();
    // test_isr2();
}
void loop() {}
/*
void setUp(void) {}
void tearDown(void) {
    //UNITY_END();
}
*/
