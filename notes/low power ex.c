// Low Power Library
#include "ArduinoLowPower.h"

// LoRa Library
#include <LoRa.h>
#include <SPI.h>

// LoRa Packet Content
char *message = "Hello LoRa!";

void setup() {
    Serial.begin(9600);
    while (!Serial)
        ;

    // LoRa Setup
    Serial.println(F("LoRa Sender"));
    if (!LoRa.begin(868E6)) {
        Serial.println(F("Starting LoRa failed!"));
        while (1)
            ;
    }
    else {
        Serial.println(F("Starting LoRa Successful!"));
    }
}

void loop() {
    LoRa_Packet_Sender();
    GoToSleep();
}

// LoRa Task
void LoRa_Packet_Sender() {
    Serial.print(F("Sending packet: "));
    Serial.println(message);

    // send packet
    LoRa.beginPacket();
    LoRa.print(message);
    LoRa.endPacket();

    // Putting LoRa Module to Sleep
    Serial.println(F("LoRa Going in Sleep"));
    LoRa.sleep();
}

// Sleep Task
void GoToSleep() {
    Serial.println(F("MKR WAN 1310 - Going in Sleep"));
    LowPower.deepSleep(20000);
}
