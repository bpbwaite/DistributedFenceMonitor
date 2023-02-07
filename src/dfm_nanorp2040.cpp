/*
  FILE: DFM_NANORP2040.CPP
  VERSION: 0.0.2
  DATE: 25 January 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite, Jack Ramsay
  DESCRIPTION: Minimal test code for RP2040Connect
*/
#include <dfm_nanorp2040.h>
#if defined(ARDUINO_NANO_RP2040_CONNECT) && !defined(CENTRAL_NODE)

#include <Arduino.h>
#include <Arduino_LSM6DSOX.h>
#include <WiFiNINA.h>



void setup_nanorp2040() {

#ifdef DEBUG
    Serial.begin(SERIALBAUD);
    while (!Serial)
        ;
#endif

    if (!IMU.begin()) {
        Serial.println("Failed to initialize IMU!");

        while (1)
            ;
    }

    Serial.print("Accelerometer sample rate = ");
    Serial.print(IMU.accelerationSampleRate());
    Serial.println(" Hz");
    Serial.println();
    Serial.println("Acceleration in g's");
    Serial.println("X\tY\tZ");
    for (;;)
        ;
}
void loop_nanorp2040() {
    float x, y, z;

    if (IMU.accelerationAvailable()) {
        IMU.readAcceleration(x, y, z);

        Serial.print(x);
        Serial.print('\t');
        Serial.print(y);
        Serial.print('\t');
        Serial.println(z);
    }
}

#endif
