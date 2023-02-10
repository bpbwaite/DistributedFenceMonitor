/*
  FILE: DFM_UTILS.CPP
  VERSION: 0.0.1
  DATE: 10 February 2023
  PROJECT: Distributed Fence Monitor Capstone
  AUTHORS: Briellyn Braithwaite
  DESCRIPTION: Functions for Distributed Fence Monitor
*/

#include "dfm_utils.h"
#include "dfm_mkr1310.h"
#include <time.h>
#include <SPI.h>
#include <LoRa.h>

void indicateOn() {
    digitalWrite(PIN_STATUSLED, HIGH);
}
void indicateOff() {
    digitalWrite(PIN_STATUSLED, LOW);
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
void epchtostr(char *p, uint32_t epc) {
    struct tm ts;
    time_t now = (time_t) (epc + GMTOFFSET);
    ts         = *localtime(&now);
    strftime(p, 22, "%m-%d-%Y %H:%M:%S", &ts);
}

void mndtostr(char *p, const MonitoringNodeData d) {

    char channelStr[20];
    int k;
    if (d.freq >= LoRaChannelsUS[0] && d.freq <= LoRaChannelsUS[NUMCHANNELS_US]) {
        for (k = 0; k < NUMCHANNELS_US; ++k)
            if (LoRaChannelsUS[k] == d.freq)
                break;
        sprintf(channelStr, "America Ch.%d", k);
    }
    else if (d.freq >= LoRaChannelsEU[0] && d.freq <= LoRaChannelsEU[NUMCHANNELS_EU]) {
        int k;
        for (k = 0; k < NUMCHANNELS_EU; ++k)
            if (LoRaChannelsEU[k] == d.freq)
                break;
        sprintf(channelStr, "Europe Ch.%d", k);
    }
    else
        strcpy(channelStr, "Unknown");

    char syncWordGoodStr[4];
    if (d.SyncWord == SYNCWORD)
        strcpy(syncWordGoodStr, "OK");
    else
        strcpy(syncWordGoodStr, "ERR");

    char epochStr[22];
    epchtostr(epochStr, d.epoch);

    char confStr[10];
    sprintf(confStr, "SF%dBW%d", SPREADFACTOR, int(CHIRPBW / 1000U));

    char rangeStr[6];
    int rssi = LoRa.packetRssi();
    int dist = 0;
    if (rssi >= -18)
        dist = 0;
    else if (rssi >= -28)
        dist = 1;
    else if (rssi >= -48)
        dist = 5;
    else if (rssi >= -55)
        dist = 10;
    else if (rssi >= -60)
        dist = 50;
    else
        dist = 5000;

    sprintf(rangeStr, "~%d m", dist);

    // snr relative strength is SF dependent
    float snr = LoRa.packetSnr();

    PGM_P format = ">ID:     0x%02X\r\n"
                   ">Pack:   %d\r\n"
                   ">Stat:   0x%02X\r\n"
                   ">Cons:   %d\r\n"
                   ">Batt:   %d\r\n"
                   ">Freq:   %.1fMHz (%s)\r\n"
                   ">SW:     0x%04X (%s)\r\n"
                   ">Uptime: %ds\r\n"
                   ">TOA:    %dms\r\n"
                   ">Temp:   %.1fC\r\n"
                   ">Acc X:  %.2fg\r\n"
                   ">Acc Y:  %.2fg\r\n"
                   ">Acc Z:  %.2fg\r\n"
                   ">Epoch:  %s\r\n"
                   ">Conf:   %s\r\n"
                   ">RSSI:   %ddBmW (%s)\r\n"
                   ">SNR:    %.1fdB";

    sprintf(p,
            format,
            d.ID,
            d.packetnum,
            d.status,
            d.connectedNodes,
            d.bat,
            d.freq / 1000000.0,
            channelStr,
            d.SyncWord,
            syncWordGoodStr,
            d.upTime / 1000,
            d.timeOnAir,
            d.temperature,
            d.accelX,
            d.accelY,
            d.accelZ,
            epochStr,
            confStr,
            rssi,
            rangeStr,
            snr);
}
