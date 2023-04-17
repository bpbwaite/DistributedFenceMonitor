/*
  FILE: DFM_UTILS.CPP
  VERSION: 0.0.5
  DATE: 1 March 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite
  DESCRIPTION: Functions for Distributed Fence Monitor
*/
#ifdef ARDUINO_SAMD_MKRWAN1310
#include "dfm_utils.h"
#include "dfm_mkr1310.h"

#include <LoRa.h>
#include <SPI.h>
#include <SparkFun_ADXL345.h>
#include <time.h>

void timeStamp() {
    Serial.print(F("-> "));
    Serial.print((millis() / 1000.0), 1);
    Serial.print(F(": "));
}
void indicateOn() {
    digitalWrite(PIN_STATUSLED, HIGH);
}
void indicateOff() {
    digitalWrite(PIN_STATUSLED, LOW);
}
void errorOn() {
    digitalWrite(PIN_ERRORLED, HIGH);
}
void errorOff() {
    digitalWrite(PIN_ERRORLED, LOW);
}
void ERROR_OUT(uint8_t sequence) {
    const unsigned long dash_time  = 1200;
    const unsigned long dot_time   = 200;
    const unsigned long slack_time = 1000;
    for (byte x = 0b10000000; x > 0; x /= 2) {
        errorOn();
        if (sequence & x)
            delay(dash_time);
        else
            delay(dot_time);
        errorOff();
        delay(dot_time);
    }
    delay(slack_time);
}

void fullResetADXL(ADXL345 *adxl) {
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

double bwCodeToFs(byte bwc) {
    double Fs = -1;

    switch (bwc) {
    case (ADXL345_BW_1600):
        Fs = 3200;
        break;
    case (ADXL345_BW_800):
        Fs = 1600;
        break;
    case (ADXL345_BW_400):
        Fs = 800;
        break;
    case (ADXL345_BW_200):
        Fs = 400;
        break;
    case (ADXL345_BW_100):
        Fs = 200;
        break;
    case (ADXL345_BW_50):
        Fs = 100;
        break;
    case (ADXL345_BW_25):
        Fs = 50;
        break;
    case (ADXL345_BW_12_5):
        Fs = 25;
        break;
    case (ADXL345_BW_6_25):
        Fs = 12.50;
        break;
    case (ADXL345_BW_3_13):
        Fs = 6.25;
        break;
    case (ADXL345_BW_1_56):
        Fs = 3.13;
        break;
    case (ADXL345_BW_0_78):
        Fs = 1.56;
        break;
    case (ADXL345_BW_0_39):
        Fs = 0.78;
        break;
    case (ADXL345_BW_0_20):
        Fs = 0.39;
        break;
    case (ADXL345_BW_0_10):
        Fs = 0.20;
        break;
    case (ADXL345_BW_0_05):
        Fs = 0.10;
        break;
    default:
        break;
    }
    return Fs;
}
int getDCOffset(ADXL345 *adxl, double t_increment) {
    // STATIC VARIABLES
    static int Z_Raw_Samples[ADXL_DC_CAPTURE]; // List of samples to use in calculating the offset
                                               // in the future, perhaps use the same memory as Z_Power_Samples
    const int raw_max = ADXL_SENSITIVITY * ADXL_LSB_PER_G_Z;
    const int raw_min = -1 * raw_max;

    unsigned long tols = 0; // local implementation of TOLS
                            // DATA COLLECTION SECTION

    timeStamp();
    Serial.print(F("Calibrating quiescent bias. Data: "));

    detachInterrupt(digitalPinToInterrupt(PIN_INTERRUPT));

    adxlMode(adxl, ADXL_COLLECTION);
    adxl->setInterrupt(ADXL345_ACTIVITY, false);  // disabling activity interrupt
    adxl->setInterrupt(ADXL345_DATA_READY, true); // enabling data ready interrupt

    int x, y, z, i = 0;
    adxl->getInterruptSource();
    while (i < ADXL_DC_CAPTURE) {
        adxl->readAccel(&x, &y, &z);
        Z_Raw_Samples[i] = z;

        tols = millis();

        while (!digitalRead(PIN_INTERRUPT) && (millis() - tols < ADXL_SAMPLE_TIMEOUT))
            // wait for the pin to go high and take sample
            ;
        i++;
    }

    // OFFSET COMPUTATION SECTION
    int bias = 0; // value to return (LSBs)

    int region_start; // used when looking at sections of data
    int region_end;   // used when looking at sections of data

    double Fs = bwCodeToFs(ADXL_BW); // Frequency of the samples

    // error checking: avoid divide by zero on read error
    if (Fs <= 0.00 || t_increment <= 0.00)
        return 0;

    int num_ranges = (int) ceil(ADXL_DC_CAPTURE / Fs / t_increment);
    int list_of_ranges[num_ranges];
    int region_samples = floor(t_increment * Fs);

    Serial.print(Fs);
    Serial.print(num_ranges);
    Serial.print(region_samples);

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

        Serial.print(list_of_ranges[n]);
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

    Serial.println();
    timeStamp();
    Serial.print(F("Got a bias of "));
    Serial.print(bias);
    Serial.println(" LSBs");

    return bias;
}

bool inactivityInDataEnd(double *zdata, double LastXSeconds, ADXL345 *adxl) {
    double Fs              = bwCodeToFs(ADXL_BW);
    int num_samples        = floor(LastXSeconds * Fs);
    double ZThresholdPower = sq((double) ADXL_ACT_THRESH * MPS2PI);
    for (int i = ADXL_SAMPLE_LENGTH - num_samples; i < ADXL_SAMPLE_LENGTH; ++i) {
        if (zdata[i] > ZThresholdPower) {
            return false;
        }
        else {
            ;
        }
    }
    return true;
}
void populateFIR(double *FIR) {

    const double omega_cutoff = (2.0 * PI * LPF_HZ);

    double t[FIRSIZE];
    double Fs  = bwCodeToFs(ADXL_BW);
    double acc = 0;
    for (int k = 0; k < FIRSIZE; ++k) {
        t[k]   = k / Fs;
        FIR[k] = exp(-1.0 * omega_cutoff * t[k]);
        acc += FIR[k];
    }

    timeStamp();
    Serial.print(F("FIR Values: "));
    for (int k = 0; k < FIRSIZE; ++k) {
        FIR[k] /= acc;
        Serial.print(FIR[k], 4);
        Serial.print(k != (FIRSIZE - 1) ? ',' : ' ');
    }
    Serial.println();
}

int getFilteredSeverity(int severityLevel, double *Z_Power_Samples, double *FIR) {
    // if the severity level passed in is greater than the one calculated,
    // we return that instead

    const int L           = ADXL_SAMPLE_LENGTH + FIRSIZE - 1;
    double data_maximum   = 0;
    int periodic_severity = 0;

    // run data through LPF:
    for (int n = 0; n < L; ++n) {
        double Y = 0;
        for (int k = 0; k <= n; ++k) {
            if ((n - k) < FIRSIZE && (k < ADXL_SAMPLE_LENGTH)) {
                Y += Z_Power_Samples[k] * FIR[n - k];
            }
        }
        if (Y > data_maximum) {
            // this will be checking for the current max energy
            data_maximum = Y;
        }
    }

    //  following for-loop should loop until thresholdZ is no longer passed
    for (int i = 0; i < NUM_THRESHES; ++i) {
        if (data_maximum < thresholdZ_logarithmic[i])
            break;
        periodic_severity++;
    }

    severityLevel = max(severityLevel, periodic_severity);
    return severityLevel;
}

uint8_t maxPayload(int region, int sf, long bw) {
    // get the max payload in bytes for different regions
    const uint8_t us_max_125[] = {242, 125, 53, 11, 0, 0};
    const uint8_t us_max_250[] = {242, 125, 53, 11, 0, 0};
    const uint8_t us_max_500[] = {222, 222, 222, 222, 109, 33};
    const uint8_t eu_max_125[] = {222, 222, 115, 51, 51, 51};
    const uint8_t eu_max_250[] = {222, 0, 0, 0, 0, 0};
    // sf offset is 7
    if (sf < 7)
        sf = 7;
    if (sf > 12)
        sf = 12;
    switch (region) {
    case 915:
        switch (bw) {
        case 125000:
            return us_max_125[sf - 7];
        case 250000:
            return us_max_250[sf - 7];
        case 500000:
            return us_max_500[sf - 7];
        default:
            break;
        }
        break;
    case 868:
        switch (bw) {
        case 125000:
            return eu_max_125[sf - 7];
        case 250000:
            return eu_max_250[sf - 7];
        default:
            break;
        }
        break;
    default:
        break;
    }
    return 0;
}
double getTOA(int ps, int sf, long bw, int plen, float cr, bool ucrc) {
    // does not support explicit header
    // does not support LDR optimization
    // auto applies SW len
    // takes packet size, sf, bw, CR, and using crc
    // returns the max time on air (ms) for such packets
    double symboltime   = (0b1 << sf) / (double) bw;
    double preambletime = (plen + 4.25) * symboltime;
    int payloadbits     = (8 * ps) - (4 * sf) + 8 + (ucrc ? 16 : 0);
    int payloadsymbols  = ceil(payloadbits / 4.0 / sf) * cr + 8;
    double payloadtime  = payloadsymbols * symboltime;

    return 1000.0 * (preambletime + payloadtime);
}

void setSeverity(MND_Compact &d, int severity) {
    if (severity < 0)
        severity = 0;
    if (severity > 15)
        severity = 15;
    d.all_states &= ~0xF0000000;
    d.all_states |= (severity << 28);
}
void setTSLC(MND_Compact &d, int min) {
    if (min < 0)
        min = 0;
    if (min > 15)
        min = 15;
    d.all_states &= ~0x0F000000;
    d.all_states |= (min << 24);
}
void setNeedRTC(MND_Compact &d, bool b) {
    d.all_states &= ~0x00800000;
    d.all_states |= (b << 23);
}
void setTemperature(MND_Compact &d, int degC) {
    if (degC < 0)
        degC = 0;
    if (degC > 127)
        degC = 127;
    d.all_states &= ~0x007F0000;
    d.all_states |= (degC << 16);
}
void setIMUBit(MND_Compact &d, bool b) {
    d.all_states &= ~0x00008000;
    d.all_states |= (b << 15);
}
void setBatt(MND_Compact &d, int percent) {
    if (percent < 0)
        percent = 0;
    if (percent > 100)
        percent = 100;
    d.all_states &= ~0x00007F00;
    d.all_states |= (percent << 8);
}
void setConnections(MND_Compact &d, int amt) {
    if (amt < 0)
        amt = 0;
    if (amt > 63)
        amt = 63;
    d.all_states &= ~0x000000FC;
    d.all_states |= (amt << 2);
}
int getSeverity(const MND_Compact &d) {
    return (d.all_states & 0xF0000000) >> 28;
}
int getTSLC(const MND_Compact &d) {
    return (d.all_states & 0x0F000000) >> 24;
}
bool getNeedRTC(const MND_Compact &d) {
    return (d.all_states & 0x00800000);
}
int getTemperature(const MND_Compact &d) {
    return (d.all_states & 0x007F0000) >> 16;
}
bool getIMUBit(const MND_Compact &d) {
    return (d.all_states & 0x00008000);
}
int getBatt(const MND_Compact &d) {
    return (d.all_states & 0x00007F00) >> 8;
}
int getConnections(const MND_Compact &d) {
    return (d.all_states & 0x000000FC) >> 2;
}

void epchtostr(char *p, uint32_t epc) {
    // char* must have size 22
    struct tm ts;
    time_t now = (time_t) (epc + GMTOFFSET);
    ts         = *localtime(&now);
    strftime(p, 22, "%m-%d-%Y %H:%M:%S", &ts);
}

#endif
