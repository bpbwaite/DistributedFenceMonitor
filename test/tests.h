#pragma once

#include <Arduino.h>
#if defined(ARDUINO_SAMD_MKRWAN1310)

#include "dfm_mkr1310.h"
#include "dfm_utils.h"

// Interrupt Service Routines:
void isr();

// Available test functions:
void test_errors(void);
void test_log_bat_SD(void);
void test_threshes(void);
void test_wake(void);
void test_detection(void);
void test_accel(void);
void test_power(void);

void test_isr2(void);

void test_stream(void);
void adtomatlab(Serial_ &s, const AccelData d);

void test_russey_mobile(void);
void test_russey_station(void);
void frssitomatlab(Serial_ &, const MonitoringNodeData, const ReceiverExtras);

#endif

#if defined(ARDUINO_NANO_RP2040_CONNECT)

void test_nanoble(void);

#endif
