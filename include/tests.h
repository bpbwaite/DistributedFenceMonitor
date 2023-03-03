#pragma once

#include <Arduino.h>
#if defined(ARDUINO_SAMD_MKRWAN1310)

// Interrupt Service Routines:
void isr();

// Available test functions:
void test_accel(void);
void test_power(void);
void test_isr2(void);

#endif

#if defined(ARDUINO_NANO_RP2040_CONNECT)

void test_nanoble(void);

#endif
