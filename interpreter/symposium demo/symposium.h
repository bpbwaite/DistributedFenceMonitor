#pragma once

#define SERIALBAUD    115200
#define SERIALTIMEOUT 10000
#define ADXL_DC_CAPTURE 300
#define ADXL_SAMPLE_TIMEOUT     50UL // ms
#define CALIBRATION_TIME_SLICE    0.50     // seconds
#define ADXL_CALIBRATION_INTERVAL 3000 // ms
#define FIRSIZE 16
#define LPF_HZ  8
#define ADXL_FALLING    1
#define ADXL_RISING     0
#define ADXL_COLLECTION 0x01
#define ADXL_SETTLING   0x02
#define ADXL_MOTION     0x04
// INERTIAL MEASUREMENT SETTINGS
#define ADXL_BW          ADXL345_BW_50
#define ADXL_SENSITIVITY 2 // 2, 4, 8, 16 (g)
#define ADXL_FULLRESBIT  1
#define ADXL_ACT_THRESH  0x0007 // 62.5 mg per increment
#define MPS2PI           0.6131 // square meters per second per increment
#define ADXL_TIME_REST   0.8
#define ADXL_LSB_PER_G_Z 256.0
#define GRAVITY          9.81
#define PIN_LORAMODE  2
#define PIN_DISCRETE  0
#define PIN_STATUSLED 6 // also shows the status of INT2
#define PIN_ERRORLED  3
#define PIN_ADXLCS1   A3 // chipselect pin for the primary accelerometer
#define PIN_ADXLCS2   A4 // chipselect pin for the secondary accelerometer
#define PIN_INTERRUPT 7
#define PIN_DHT       4
#define ADXL_VOLTAGE  303 // 303 for 3.3, 205 for 2.5

volatile unsigned long TOLC = 0; // time of last calibration
inline unsigned long TSLC() {
    return millis() - TOLC;
} // time since last calibration


typedef struct {
    int x;
    int y;
    int z;

} AccelData;

void accel_to_mat(Serial_ &s, const AccelData d) {
    for (unsigned int nb = 0; nb < sizeof(AccelData); ++nb)
        s.write((unsigned char) ((uint8_t *) &d)[nb]);
}


void adxlMode(ADXL345 *adxl, uint8_t mode) {
    switch (mode) {
    case ADXL_COLLECTION:
        adxl->setInterrupt(ADXL345_ACTIVITY, false);
        adxl->setInterrupt(ADXL345_INACTIVITY, false);
        adxl->setInterrupt(ADXL345_DATA_READY, true);
        break;
    case ADXL_SETTLING:
        adxl->setInterrupt(ADXL345_ACTIVITY, false);
        adxl->setInterrupt(ADXL345_INACTIVITY, true);
        adxl->setInterrupt(ADXL345_DATA_READY, false);
        break;
    case ADXL_MOTION:
        adxl->setInterrupt(ADXL345_ACTIVITY, true);
        adxl->setInterrupt(ADXL345_INACTIVITY, false);
        adxl->setInterrupt(ADXL345_DATA_READY, false);
        break;
    default:
        break;
    }
}
int getDCOffset(ADXL345 *adxl, double t_increment) {
    // STATIC VARIABLES
    static int Z_Raw_Samples[ADXL_DC_CAPTURE]; // List of samples to use in calculating the offset
                                               // in the future, perhaps use the same memory as Z_Power_Samples
    const int raw_max = ADXL_SENSITIVITY * ADXL_LSB_PER_G_Z;
    const int raw_min = -1 * raw_max;

    unsigned long tols = 0; // local implementation of TOLS
                            // DATA COLLECTION SECTION

    Serial.print(F("Calibrating quiescent bias..."));

    detachInterrupt(digitalPinToInterrupt(PIN_INTERRUPT));

    adxlMode(adxl, ADXL_COLLECTION);

    int x, y, z, i = 0;
    adxl->getInterruptSource();
    while (i < ADXL_DC_CAPTURE) {
        adxl->readAccel(&x, &y, &z);
        Z_Raw_Samples[i] = z;

        tols = millis();

        while (!digitalRead(PIN_INTERRUPT) && (millis() - tols < ADXL_SAMPLE_TIMEOUT))
            // wait for the pin to go high and take sample
            ;
        // todo: some action on watchdog (tols) timeout
        i++;
    }

    // OFFSET COMPUTATION SECTION
    int bias = 0; // value to return (LSBs)

    int region_start; // used when looking at sections of data
    int region_end;   // used when looking at sections of data

    double Fs = 100;

    // error checking: avoid divide by zero on read error
    if (Fs <= 0.00 || t_increment <= 0.00)
        return 0;

    int num_ranges = (int) ceil(ADXL_DC_CAPTURE / Fs / t_increment);
    int list_of_ranges[num_ranges];
    int region_samples = floor(t_increment * Fs);

    // find the max and min of each region to get the range
    for (int n = 0; n < num_ranges; ++n) {
        region_start = floor(n * region_samples);
        region_end   = min(region_start + region_samples, ADXL_DC_CAPTURE - 1);

        // find the min/max over the current region
        int zmax = raw_min, zmin = raw_max;
        for (int m = region_start; m < region_end; ++m) {
            if (Z_Raw_Samples[m] > zmax)
                zmax = Z_Raw_Samples[m];
            if (Z_Raw_Samples[m] < zmin)
                zmin = Z_Raw_Samples[m];
        }

        list_of_ranges[n] = zmax - zmin;
    }
    // find the value of the quiestest region
    int rmin = raw_max;
    for (int n = 0; n < num_ranges; ++n) {
        if (list_of_ranges[n] < rmin)
            rmin = list_of_ranges[n];
    }
    // find the index of that value
    int r_quiet = 0;
    for (r_quiet = 0; r_quiet < num_ranges; ++r_quiet) {
        if (list_of_ranges[r_quiet] == rmin)
            break;
    }
    //  find the average of that region
    region_start    = r_quiet * region_samples;
    region_end      = min(region_start + region_samples, ADXL_DC_CAPTURE - 1);
    int accumulator = 0;
    for (int m = region_start; m < region_end; ++m) {
        accumulator += Z_Raw_Samples[m];
    }

    bias = accumulator / max(region_samples, 1);

    return bias;
}
