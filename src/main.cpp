#include <Arduino.h>

#include <dfm_mkr1310.h>
#include <dfm_nanorp2040.h>

void setup() {
    // put your setup code here, to run once:
#ifdef BUILD_MKR1310
    setup_mkr1310();
#endif
#ifdef BUILD_NANORP2040
    setup_nanorp2040();
#endif
}

void loop() {
    // put your main code here, to run repeatedly:
#ifdef BUILD_MKR1310
    loop_mkr1310();
#endif
#ifdef BUILD_NANORP2040
    loop_nanorp2040();
#endif
}
