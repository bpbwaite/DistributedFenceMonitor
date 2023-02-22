#include "2.h"
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

MonitoringNodeData mnd_test;

void setUp(void) {
    // set stuff up here
    pinMode(PIN_STATUSLED, OUTPUT);
    indicateOff();
}

void tearDown(void) {
    // clean stuff up here
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
void setup() {
    delay(2000);

    runUnityTests();
}
void loop() {}
