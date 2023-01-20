/*
  FILE: DFM_NANORP2040.CPP
  VERSION: 0.0.2
  DATE: 20 January 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite
  DESCRIPTION: Minimal test code for RP2040Connect
*/
#include <dfm_nanorp2040.h>

#include <Arduino.h>
#include <Arduino_LSM6DSOX.h>
#include <WiFiNINA.h>

void setup_nanorp2040() {
    pinMode(LEDR, OUTPUT);
    pinMode(LEDG, OUTPUT);
    pinMode(LEDB, OUTPUT);
    digitalWrite(LEDR, LOW);
    digitalWrite(LEDG, LOW);
    digitalWrite(LEDB, LOW);
}
void loop_nanorp2040() {
    digitalWrite(LEDR, HIGH);
    delay(100);
    digitalWrite(LEDR, LOW);
    delay(100);
    digitalWrite(LEDG, HIGH);
    delay(100);
    digitalWrite(LEDG, LOW);
    delay(100);
    digitalWrite(LEDB, HIGH);
    delay(100);
    digitalWrite(LEDB, LOW);
    delay(100);
}
