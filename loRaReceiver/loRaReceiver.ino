// Charles Nicholson, 2023.
// Code for LittleLoRa tracker ground station receiver.
// Meant to be used to track a high-altitude glider.

#include "headers/settings.h"
#include <LoRa.h>
#include <SPI.h>

// NOTE: system operates at 433 mHz, so a ham radio license is needed in the US.
// Please check local regulations before use.

// ***** LoRa *****
// LoRa API reference:
// https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md. You can
// use any SX1278 module, such as the DRF1278F. Note that the pinout between
// similar boards is different.
// Wiring of the SX1278 module:
// NSS of SX to ATN (CS, D38, physical pin 22).
// Reset of SX to 5 (physical pin 24).
// DIO0 of SX to 2 (physical pin 23).
// VCC of SX to 3.3v.
// GND of SX to GND.
// MOSI of SX to 11 (physical pin 19).
// MISO of SX to 12 (physical pin 21).
// SCK of SX to 13 (physical pin 20).

// ***** To-Do *****
// - Configure the 32 bit data types to work well with the 64 bit Python that is getting sent the data over Pyserial.

struct __attribute__((packed)) dataStruct {
  float lat;
  float lon;
  short volts;
  short alt;
  byte sats;
  byte speed;
  short course;
  byte seconds;
  byte minutes;
  byte hours;
  short txCount;
  char text[7];
} payload;

void setup() {
  Serial.begin(BAUD_RATE);
  while (!Serial)
    ;

#ifdef DEVMODE
  Serial.println("LittleLoRa Receiver.");
#endif

  LoRa.setPins(SS_PIN, RESET_PIN, DIO0_PIN); // Has to be before LoRa.begin().

  if (!LoRa.begin(FREQUENCY)) {
#ifdef DEVMODE
    Serial.println("Starting LoRa failed!");
#endif
    while (1)
      ;
  }

  LoRa.setSyncWord(SYNC_WORD);               // Defined in settings.h.
  LoRa.setSpreadingFactor(SPREADING_FACTOR); // Defined in settings.h.
  LoRa.setSignalBandwidth(BANDWIDTH);        // Defined in settings.h.
  LoRa.crc();                                // Use a checksum.

#ifdef DEVMODE
  Serial.println("LoRa initialized, starting in 1 second.");
#endif

  delay(1000);
}

void loop() {
  int packetSize = LoRa.parsePacket(); // Parse packet.
  if (packetSize > 0) {
    LoRa.readBytes((byte *)&receivedData, sizeof(receivedData)); // Receive packet and put it into a struct.

    // Check if the packet is a valid packet.
    if (sizeof(receivedData) == packetSize) {
#ifdef DEVMODE
      displayData();
#endif
#ifndef DEVMODE
      long rssi = LoRa.packetRssi();
      long snr = LoRa.packetSnr();

      Serial.write((uint8_t *)&receivedData.lat, sizeof(float)); // Send data over serial to the Python SondeHub uploader.
      Serial.write((uint8_t *)&receivedData.lng, sizeof(float));
      Serial.write((uint8_t *)&receivedData.alt, sizeof(long));
      Serial.write((uint8_t *)&receivedData.sats, sizeof(long));
      Serial.write((uint8_t *)&receivedData.speed, sizeof(long));
      Serial.write((uint8_t *)&receivedData.course, sizeof(long));
      Serial.write((uint8_t *)&receivedData.txCnt, sizeof(long));
      Serial.write((uint8_t *)&rssi, sizeof(long));
      Serial.write((uint8_t *)&snr, sizeof(long));
#endif
    }
  }
}