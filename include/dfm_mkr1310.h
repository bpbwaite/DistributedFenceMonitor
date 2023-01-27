#pragma once

#ifdef ARDUINO_SAMD_MKRWAN1310

#define SERIALBAUD   115200
#define LORA_AMERICA 915E6
#define LORA_AFRICA  868E6
#define LORA_EUROPE  433E6

typedef struct MonitoringNodeData {
    uint16_t myID;
    uint32_t packetnumber;
    byte status;
    uint16_t connectedNodes;
    byte batLevel;
    byte freq;
    float SNR;
    byte SFSyncword;
    uint32_t upTime;
    byte temperature;
};

void setup_mkr1310();
void loop_mkr1310();
#endif
