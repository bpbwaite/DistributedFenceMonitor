/*
  FILE: MAIN.CPP
  VERSION: 0.5.0
  DATE: 19 January 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite
  DESCRIPTION:
*/

#include <Arduino.h>

#include <dfm_mkr1310.h>
#include <dfm_recv_mkr1310.h>
#include <dfm_nanorp2040.h>
#include <dfm_recv_nanorp2040.h>

void setup() {
    // put your setup code here, to run once:
#ifdef BUILD_MKR1310
    setup_mkr1310();
#endif
#ifdef BUILD_RECV_MKR1310
    setup_recv_mkr1310();
#endif
#ifdef BUILD_NANORP2040
    setup_nanorp2040();
#endif
#ifdef BUILD_RECV_NANORP2040
    setup_recv_nanorp2040();
#endif
}

void loop() {
    // put your main code here, to run repeatedly:
#ifdef BUILD_MKR1310
    loop_mkr1310();
#endif
#ifdef BUILD_RECV_MKR1310
    loop_recv_mkr1310();
#endif
#ifdef BUILD_NANORP2040
    loop_nanorp2040();
#endif
#ifdef BUILD_RECV_NANORP2040
    loop_recv_nanorp2040();
#endif
}
