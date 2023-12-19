#include <Arduino.h>
#include "headers/settings.h"

void displayData() {
  Serial.print("Lat: ");
  Serial.print(payload.lat, 9);
  Serial.print(" Lon: ");
  Serial.print(payload.lon, 9);
  Serial.print(" Alt: ");
  Serial.print(payload.alt);
  Serial.print(" Date/Time: ");
  Serial.print(year);
  Serial.print("-");
  Serial.print(month);
  Serial.print("-");
  Serial.print(day);
  Serial.print(" ");
  Serial.print(payload.hours);
  Serial.print(":");
  Serial.print(payload.minutes);
  Serial.print(":");
  Serial.print(payload.seconds);
  Serial.print(" Fix type: ");
  Serial.print(fixType);
  Serial.print(" Sats: ");
  Serial.print(payload.sats);
  Serial.print(" Transmissions: ");
  Serial.print(payload.txCount);
  Serial.print(" Voltage: ");
  Serial.print(payload.volts);
  Serial.print(" Speed: ");
  Serial.print(payload.speed);
  Serial.print(" Course: ");
  Serial.print(payload.course);
  Serial.print(" SNR: ");
  Serial.print(LoRa.packetSnr());
  Serial.print(" RSSI: ");
  Serial.print(LoRa.packetRssi());
  Serial.print(" Callsign: ");
  Serial.println(payload.comment);
}