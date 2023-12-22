#include <Arduino.h>
#include "headers/settings.h"

void displayData() {
  Serial.print("Lat: ");
  Serial.print(receivedData.lat, 9);
  Serial.print(" Lon: ");
  Serial.print(receivedData.lon, 9);
  Serial.print(" Alt: ");
  Serial.print(receivedData.alt);
  Serial.print(" Time: ");
  Serial.print(receivedData.hours);
  Serial.print(":");
  Serial.print(receivedData.minutes);
  Serial.print(":");
  Serial.print(receivedData.seconds);
  Serial.print(" Sats: ");
  Serial.print(receivedData.sats);
  Serial.print(" Transmissions: ");
  Serial.print(receivedData.txCount);
  Serial.print(" Receptions: ");
  Serial.print(rxCount);
  Serial.print(" Voltage: ");
  Serial.print(receivedData.volts);
  Serial.print(" Speed: ");
  Serial.print(receivedData.speed);
  Serial.print(" Course: ");
  Serial.print(receivedData.course);
  Serial.print(" SNR: ");
  Serial.print(LoRa.packetSnr());
  Serial.print(" RSSI: ");
  Serial.print(LoRa.packetRssi());
  Serial.print(" Callsign: ");
  Serial.println(receivedData.text);
}