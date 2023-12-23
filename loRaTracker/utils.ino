#include <Arduino.h>
#include "headers/settings.h"

void longPulse() {
  digitalWrite(LED, HIGH);
  delay(250);
  digitalWrite(LED, LOW);
  delay(500);
  digitalWrite(LED, HIGH);
  delay(250);
  digitalWrite(LED, LOW);
}

void shortPulse() {
  digitalWrite(LED, HIGH);
  delay(250);
  digitalWrite(LED, LOW);
}

void readVoltage() {
  int rawVolt = analogRead(BAT_VOLTAGE_PIN);
  rawVolt = rawVolt * 2;
  payload.volts = rawVolt * (3.3 / 1023.0) * 100;
}

void displayData() {
  SerialUSB.print("Lat: ");
  SerialUSB.print(payload.lat, 9);
  SerialUSB.print(" Lon: ");
  SerialUSB.print(payload.lon, 9);
  SerialUSB.print(" Alt: ");
  SerialUSB.print(payload.alt);
  SerialUSB.print(" Date/Time: ");
  SerialUSB.print(year);
  SerialUSB.print("-");
  SerialUSB.print(month);
  SerialUSB.print("-");
  SerialUSB.print(day);
  SerialUSB.print(" ");
  SerialUSB.print(payload.hours);
  SerialUSB.print(":");
  SerialUSB.print(payload.minutes);
  SerialUSB.print(":");
  SerialUSB.print(payload.seconds);
  SerialUSB.print(" Fix type: ");
  SerialUSB.print(fixType);
  SerialUSB.print(" Sats: ");
  SerialUSB.print(payload.sats);
  SerialUSB.print(" Transmissions: ");
  SerialUSB.print(payload.txCount);
  SerialUSB.print(" Voltage: ");
  SerialUSB.print(payload.volts);
  SerialUSB.print(" Speed: ");
  SerialUSB.print(payload.speed);
  SerialUSB.print(" Course: ");
  SerialUSB.print(payload.course);
  SerialUSB.print(" Comment: ");
  SerialUSB.println(payload.text);
}

void getData() {
  payload.lat = gps.getLatitude();
  payload.lat = payload.lat / 10000000;
  payload.lon = gps.getLongitude();
  payload.lon = payload.lon / 10000000;
  payload.alt = gps.getAltitude();
  payload.alt = payload.alt / 1000;
  payload.sats = gps.getSIV();
  payload.speed = gps.getGroundSpeed();
  payload.course = gps.getHeading();
  payload.seconds = gps.getSecond();
  payload.minutes = gps.getMinute();
  payload.hours = gps.getHour();
  fixType = gps.getFixType();
  day = gps.getDay();
  month = gps.getDay();
  year = gps.getYear();
}

void gpsWakeup() {
  digitalWrite(WAKEUP_PIN, LOW);
  delay(1000);
  digitalWrite(WAKEUP_PIN, HIGH);
  delay(1000);
  digitalWrite(WAKEUP_PIN, LOW);
}

void gpsConfig() {
  gps.factoryDefault(); // Clear any saved configuration

  bool setValueSuccess = true;

  gps.enableGNSS(true, SFE_UBLOX_GNSS_ID_GPS);      // Enable GPS, disable everything else for lower power and for PSMOO to work.
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_SBAS);    // Disable SBAS.
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_GALILEO); // Disable Galileo.
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_BEIDOU);  // Disable BeiDou.
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_IMES);    // Disable IMES.
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_QZSS);    // Disable QZSS.
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_GLONASS); // Disable GLONASS.

  setValueSuccess &= gps.setVal8(UBLOX_CFG_PM_OPERATEMODE, 1);       // Setting to PSMOO.
  setValueSuccess &= gps.setVal32(UBLOX_CFG_PM_POSUPDATEPERIOD, 60); // Wakes up every ~30 seconds to get a fix in PSMOO mode.
  setValueSuccess &= gps.setVal32(UBLOX_CFG_PM_ACQPERIOD, 10);       // 10 seconds between aquicistion attempts if no fix after MAXACQTIME. Ignored because DONOTENTEROFF is true.
  setValueSuccess &= gps.setVal16(UBLOX_CFG_PM_ONTIME, 0);           // Set to go straight to low power mode after fix.
  setValueSuccess &= gps.setVal8(UBLOX_CFG_PM_MINACQTIME, 5);        // Spend at least 5 seconds getting a fix even if the signal is bad. Ignored because DONOTENTEROFF is true.
  setValueSuccess &= gps.setVal8(UBLOX_CFG_PM_MAXACQTIME, 15);       // Spend at maximum 15 seconds getting a fix. Ignored because DONOTENTEROFF is true.
  setValueSuccess &= gps.setVal8(UBLOX_CFG_PM_DONOTENTEROFF, 1);     // GPS won't go to low power mode if there is no fix.
  setValueSuccess &= gps.setVal8(UBLOX_CFG_PM_WAITTIMEFIX, 1);       // In the acquistion phase, a time fix is waited for.
  setValueSuccess &= gps.setVal8(UBLOX_CFG_PM_UPDATEEPH, 1);         // Update the ephemeris (satelitte data) regularly.
  setValueSuccess &= gps.setVal8(UBLOX_CFG_PM_EXTINTBACKUP, 0);      // Disable EXTINT backup.
  setValueSuccess &= gps.setVal8(UBLOX_CFG_PM_EXTINTWAKE, 0);        // Disable EXTINT wake.
  setValueSuccess &= gps.setVal8(UBLOX_CFG_PM_EXTINTINACTIVE, 0);    // Disable EXTINT active.
  setValueSuccess &= gps.setVal32(UBLOX_CFG_PM_EXTINTINACTIVITY, 0); // Disable EXTINT activity.

  if (setValueSuccess == true) {
    gps.saveConfiguration();
#ifdef DEVMODE
    SerialUSB.println("GPS Config Success!");
#endif
  } else {
#ifdef DEVMODE
    SerialUSB.println("GPS Config Failed!");
#endif
  }
}

void transmit(const struct __attribute__((packed)) dataStruct &data) {
  // Send the packet over LoRa.
  LoRa.beginPacket();
  LoRa.write((byte *)&data, sizeof(data));
  LoRa.endPacket();
}

/*
void sendI2CData(byte data[]) {
  Wire.beginTransmission(0x42); // Start transmission to GPS.
  for (int i = 0; i < sizeof(data); i++) {
    Wire.write(data[i]); // Send data byte by byte.
  }
  Wire.endTransmission(); // End transmission.
}


void gpsSleep() {
  // Sleep for 10 seconds
  byte UBLOX_RXM_PMREQ[] = {
    0xB5, 0x62, 0x02, 0x41, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27,
0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0xF0, 0x38
  };
  sendI2CData(UBLOX_RXM_PMREQ);
}
*/