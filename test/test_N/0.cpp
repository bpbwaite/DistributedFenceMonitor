#include "0.h"
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

void setUp(void) {
    // set stuff up here
    pinMode(PIN_STATUSLED, OUTPUT);
    indicateOff();
}

void tearDown(void) {
    // clean stuff up here
}

void test_function(void) {}

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
