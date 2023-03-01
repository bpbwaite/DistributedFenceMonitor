#include "0.h"
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

void test_function(void) {
    // put most of the code here
    pinMode(PIN_STATUSLED, OUTPUT);
    indicateOff();
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
