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
#include <LoRa.h>
#include <SPI.h>
#include <time.h>

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
int getSeverity(MND_Compact &d) {
    return (d.all_states & 0xF0000000) >> 28;
}
int getTSLC(MND_Compact &d) {
    return (d.all_states & 0x0F000000) >> 24;
}
bool getNeedRTC(MND_Compact &d) {
    return (d.all_states & 0x00800000);
}
int getTemperature(MND_Compact &d) {
    return (d.all_states & 0x007F0000) >> 16;
}
bool getIMUBit(MND_Compact &d) {
    return (d.all_states & 0x00008000);
}
int getBatt(MND_Compact &d) {
    return (d.all_states & 0x00007F00) >> 8;
}
int getConnections(MND_Compact &d) {
    return (d.all_states & 0x000000FC) >> 2;
}

void epchtostr(char *p, uint32_t epc) {
    // char* must have size 22
    struct tm ts;
    time_t now = (time_t) (epc + GMTOFFSET);
    ts         = *localtime(&now);
    strftime(p, 22, "%m-%d-%Y %H:%M:%S", &ts);
}

// void mndtostr(Serial_ &s, const MonitoringNodeData d) {
//     static char p[322];
//
//     char channelStr[20];
//     int k;
//     if ((d.freq >= LoRaChannelsUS[0]) && (d.freq <= LoRaChannelsUS[NUMCHANNELS_US - 1])) {
//         for (k = 0; k < NUMCHANNELS_US; ++k)
//             if (LoRaChannelsUS[k] == (long) d.freq)
//                 break;
//         sprintf(channelStr, "America Ch. %d", k);
//     }
//     else if ((d.freq >= LoRaChannelsEU[0]) && (d.freq <= LoRaChannelsEU[NUMCHANNELS_EU - 1])) {
//         int k;
//         for (k = 0; k < NUMCHANNELS_EU; ++k)
//             if (LoRaChannelsEU[k] == (long) d.freq)
//                 break;
//         sprintf(channelStr, "Europe Ch. %d", k);
//     }
//     else
//         strcpy(channelStr, "Unknown");
//
//     char syncWordGoodStr[4];
//     if (d.SyncWord == SYNCWORD)
//         strcpy(syncWordGoodStr, "OK");
//     else
//         strcpy(syncWordGoodStr, "ERR");
//
//     char epochStr[22];
//     epchtostr(epochStr, d.epoch);
//
//     char confStr[10];
//     sprintf(confStr, "SF%dBW%d", SPREADFACTOR, int(CHIRPBW / 1000U));
//
//     // snr relative strength is SF dependent
//     float snr = LoRa.packetSnr();
//
//     PGM_P format = ">ID: 0x%02X\r\n"
//                    ">Pack: %d\r\n"
//                    ">Stat: 0x%02X\r\n"
//                    ">Cons: %d\r\n"
//                    ">Batt: %d\r\n"
//                    ">Freq: %.1fMHz (%s)\r\n"
//                    ">SW: 0x%04X (%s)\r\n"
//                    ">Uptime: %ds\r\n"
//                    ">TOA: %dms\r\n"
//                    ">Temp: %.1fC\r\n"
//                    ">AccX: %.2fg\r\n"
//                    ">AccY: %.2fg\r\n"
//                    ">AccZ: %.2fg\r\n"
//                    ">Epoch: %s\r\n"
//                    ">Conf: %s\r\n"
//                    ">RSSI: %ddBmW\r\n"
//                    ">SNR: %.1fdB\r\n";
//
//     // use to determine the max format buffer size required, then comment out.
//     // remember to leave one for null char
//     // PGM_P formatmax = ">ID: 0xAA\r\n"
//     //                   ">Pack: 1000000000\r\n"
//     //                   ">Stat: 0xAA\r\n"
//     //                   ">Cons: 1000000000\r\n"
//     //                   ">Batt: 1000000000\r\n"
//     //                   ">Freq: 999.1fMHz (America Ch. 64)\r\n"
//     //                   ">SW: 0xAAAA (ERR)\r\n"
//     //                   ">Uptime: 1000000000s\r\n"
//     //                   ">TOA: 1000000000ms\r\n"
//     //                   ">Temp: 254.12C\r\n"
//     //                   ">AccX: -15.25g\r\n"
//     //                   ">AccY: -15.25g\r\n"
//     //                   ">AccZ: -15.25g\r\n"
//     //                   ">Epoch: 12-31-2000 24:59:59\r\n"
//     //                   ">Conf: SF12BW500\r\n"
//     //                   ">RSSI: -120dBmW\r\n"
//     //                   ">SNR: -20.0dB\r\n";
//
//     sprintf(p,
//             format,
//             d.ID,
//             d.packetnum,
//             d.status,
//             d.connectedNodes,
//             d.bat,
//             d.freq / 1000000.0,
//             channelStr,
//             d.SyncWord,
//             syncWordGoodStr,
//             d.upTime / 1000,
//             d.timeOnAir,
//             d.temperature,
//             0,
//             0,
//             0,
//             epochStr,
//             confStr,
//             LoRa.packetRssi(),
//             snr);
//
//     s.print(p);
// }
// void mndtomatlab(Serial_ &s, const MonitoringNodeData d, const ReceiverExtras e) {
//     for (int nb = 0; nb < sizeof(MonitoringNodeData); ++nb)
//         s.write((unsigned char) ((uint8_t *) &d)[nb]);
//     for (int nb = 0; nb < sizeof(ReceiverExtras); ++nb)
//         s.write((unsigned char) ((uint8_t *) &e)[nb]);
// }

#endif
