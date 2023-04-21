#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <RTCZero.h>
#include <ArduinoLowPower.h>
#include <SparkFun_ADXL345.h>

#include "symposium.h"

ADXL345 *adxl;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(SERIALBAUD);
      while (!Serial && millis() < SERIALTIMEOUT)
          yield();
          
          
    pinMode(PIN_INTERRUPT, INPUT_PULLDOWN);
    pinMode(PIN_STATUSLED, INPUT);
    pinMode(PIN_ERRORLED, OUTPUT);

  adxl = new ADXL345(PIN_ADXLCS1);

  adxl->powerOn();
  adxl->setSpiBit(0); // allow 4-Wire SPI from ADXL to main
  adxl->setRangeSetting(ADXL_SENSITIVITY);
  adxl->setFullResBit(ADXL_FULLRESBIT);
  adxl->set_bw(ADXL_BW);
  adxl->setInterruptLevelBit(ADXL_RISING); // means the pin RISES on interrupt

  adxl->setActivityAc(1); // AC coupled activitiy
  adxl->setActivityXYZ(0, 0, 1);
  adxl->setActivityThreshold(ADXL_ACT_THRESH);
  adxl->setInactivityAc(1);
  adxl->setInactivityXYZ(0, 0, 1);
  adxl->setInactivityThreshold(ADXL_ACT_THRESH);
  adxl->setTimeInactivity(ADXL_TIME_REST);

  adxl->ActivityINT(1);
  adxl->InactivityINT(0);
  adxl->FreeFallINT(0);
  adxl->doubleTapINT(0);
  adxl->singleTapINT(0);

  // disable FIFO-related interrupts
  adxl->setInterrupt(ADXL345_OVERRUNY, false);
  adxl->setInterrupt(ADXL345_WATERMARK, false);
  adxl->setInterrupt(ADXL345_DATA_READY, false);

  // initial int map
  adxl->setInterruptMapping(ADXL345_ACTIVITY, ADXL345_INT1_PIN);
  adxl->setInterruptMapping(ADXL345_INACTIVITY, ADXL345_INT1_PIN);
  adxl->setInterruptMapping(ADXL345_DATA_READY, ADXL345_INT1_PIN);

  adxlMode(adxl, ADXL_COLLECTION);
}

int DC_offset = 0;

void loop() {
  // put your main code here, to run repeatedly:
  static AccelData ad;


 // check if we should recompute the DC offset
  if (TSLC() > ADXL_CALIBRATION_INTERVAL) {
      DC_offset     = getDCOffset(adxl, CALIBRATION_TIME_SLICE);
      TOLC          = millis();
  }

  if (digitalRead(PIN_INTERRUPT)) {
            
            adxl->readAccel(&ad.x, &ad.y, &ad.z);
            ad.z -= DC_offset;
            accel_to_mat(Serial, ad);
            Serial.println();
        }

}
