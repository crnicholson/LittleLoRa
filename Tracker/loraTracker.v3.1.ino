// Charles Nicholson, 2023
// Code for a LoRa GPS tracker.
// Meant to be used to track a high-altitude glider.
// https://github.com/crnicholson/littleLoRa/tree/main

// ***** NOTE *****
// This system operates at 433 MHz, so a ham radio license is needed in the US.
// Please check local regulations before use. Please also input your callsign in the settings.h file.

// ***** GPS *****
// This tracker takes advantage of the special abilities of the u-blox MAX-M10S module.
// If you choose to modifiy this code to use a different GPS, a different GPS library will need to be used.
// Furthermore, it is extremely likely you will lose the ability to enter power saving mode.

// ***** GPS Low Power *****
// There are two pins of the module you can use to wakeup the module:
// VAL_RXM_PMREQ_WAKEUPSOURCE_UARTRX  = uartrx
// VAL_RXM_PMREQ_WAKEUPSOURCE_EXTINT0 = extint0 (default)
// These values can be or'd (|) together to enable interrupts on multiple pins.
// On the SAMD side, a digital pin is needed that is capable of driving HIGH and LOW. In this case, we use the digital pin 8 of the SAMD.
// More information on the configuration of the power save mode can be found in "Example22_PowerOff" of the Sparkfun GNSS v3 library.

// ***** SAMD21G18A *****
// This tracker uses a SAMD21G18A microcontroller because of its many benefits. The SAMD needs a bootloader burned onto it when you get it from Digikey.
// My friend is working on a tutorial for burning a bootloader. The bootloader burns as an Arduino Zero, native USB port.
// The pin definitions are for the Arduino Zero, which is essentially the same as an UNO. If you wish not to use the SAMD, your life will become much harder.
// Trust me. Just use it. Forget about the ancient ATMega328.

// ***** SAMD21G18A Low Power *****
// The SAMD goes to sleep to save power. This is implmented using the ArduinoLowPower library.
// With the LoRa, MCU, and ublox in sleep, plus the MCP1700 voltage regulator, the current draw can be as low as 280 uA while retaining function. 
// To achieve proper sleep, some edits to the SAMD core are neccesary. To find the wiring.c file on your computer, follow this guide:
// https://support.arduino.cc/hc/en-us/articles/4415103213714-Find-sketches-libraries-board-cores-and-other-files-on-your-computer. 
// Once there, comment out this line as shown:
/*
  // Setup all pins (digital and analog) in INPUT mode (default is nothing)
  for (uint32_t ul = 0 ; ul < NUM_DIGITAL_PINS ; ul++ )
  {
    pinMode( ul, INPUT ) ;
  }
*/
// Arduino gives you an official warning, but from my testing, it doesn't effect anything:
// "This breaks Arduino APIs since all pins are considered INPUT at startup.
// However, it really lowers the power consumption by a factor of 20 in low power mode (0.03mA vs 0.6mA)"

// ***** LoRa *****
// LoRa API reference: https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md.
// You can use any SX1278 module, such as the DRF1278F.
// Note that the pinout between similar boards is different.
// Wiring of the SX1278 module:
// NSS of SX to ATN (CS, D38, physical pin 22).
// Reset of SX to 5 (physical pin 24).
// DIO0 of SX to 2 (physical pin 23).
// VCC of SX to 3.3v.
// GND of SX to GND.
// MOSI of SX to 11 (physical pin 19).
// MISO of SX to 12 (physical pin 21).
// SCK of SX to 13 (physical pin 20).

// For finding LoRa link budget and transmit time, follow this link:
// https://www.semtech.com/design-support/lora-calculator.
// Above link also includes other important LoRa factors.

// If you don't want to use LoRa for some odd reason, there are other options.
// One option is to use my friend's tracker, Tiny4FSK. Link here:
// https://github.com/New-England-Weather-Balloon-Society/Tiny4FSK/tree/main

#include <SPI.h>                      // Needed for LoRa.
#include <LoRa.h>                     // LoRa.
#include <Wire.h>                     // Needed for I2C to GPS module.
#include <SparkFun_u-blox_GNSS_v3.h>  // http://librarymanager/All#SparkFun_u-blox_GNSS_v3.
#include <ArduinoLowPower.h>
#include "settings.h"  // Change settings here.

struct __attribute__((packed)) dataStruct {
  float lat;
  float lon;
  long alt;
  long sats;
  long speed;
  long course;
  long txCount = 0;
  char text[50] = comment;
} transmittingData;

#ifdef STATIC_COORD
float lat = 42.316651;
float lon = -71.366030;
int alt = 96;
int sats = 9;
int speed = 60;
int course = 360;
#endif

SFE_UBLOX_GNSS gps;

void setup() {
  pinMode(WAKEUP_PIN, OUTPUT);
  digitalWrite(WAKEUP_PIN, LOW);

#ifdef DEVMODE
  SerialUSB.begin(BAUD_RATE);
  while (!SerialUSB)
    ;  // Wait until SerialUSB is all good.
  SerialUSB.println("LoRa Tracker v2.1");
#endif

  LoRa.setPins(SS_PIN, RST_PIN, DIO0PIN);  // SS, reset, and DIO0. Has to be before LoRa.begin().

  if (!LoRa.begin(FREQ)) {
#ifdef DEVMODE
    SerialUSB.println("Starting LoRa failed!");
#endif
    while (1)
      ;
  }

  LoRa.setSyncWord(SYNC_WORD);
  LoRa.setSpreadingFactor(SPREADING_FACTOR);
  LoRa.setSignalBandwidth(BANDWIDTH);
  LoRa.crc();

  Wire.begin();

  if (gps.begin() == false) {  // Connect to the u-blox module using Wire port.
#ifdef DEVMODE
    SerialUSB.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
#endif
    while (1)
      ;
  }

  /*
  gps.enableGNSS(true, SFE_UBLOX_GNSS_ID_GPS); // Make sure GPS is enabled (we must leave at least one major GNSS enabled!)
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_SBAS); // Disable SBAS
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_GALILEO); // Disable Galileo
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_BEIDOU); // Disable BeiDou
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_IMES); // Disable IMES
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_QZSS); // Disable QZSS
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_GLONASS); // Disable GLONASS
  gps.factoryDefault();  // Clear any saved configuration
  gps.hardReset();  // Hard reset - force a cold start
  */

  gpsConfig();

  // Wait for 5 seconds
#ifdef DEVMODE
  SerialUSB.println("Waiting for 10 seconds before loop.");
#endif
  for (int i = 0; i < 10; i++) {
    delay(1000);
#ifdef DEVMODE
    SerialUSB.print(".");
#endif
  }
#ifdef DEVMODE
  SerialUSB.println();
#endif
}

void loop() {
#ifdef STATIC_COORD
  transmittingData.lat = lat;
  transmittingData.lon = lon;
  transmittingData.alt = alt;
  transmittingData.sats = sats;
  transmittingData.speed = speed;
  transmittingData.course = course;
  transmittingData.txCount++;
#endif

#ifndef STATIC_COORD
  if (gps.getPVT()) {
    transmittingData.lat = gps.getLatitude();
    transmittingData.lon = gps.getLongitude();
    transmittingData.alt = gps.getAltitude();
    transmittingData.sats = gps.getSIV();
    transmittingData.speed = 0;
    transmittingData.course = 0;
    transmittingData.txCount++;
    transmittingData.lat = transmittingData.lat / 10000000;  // Convert to float.
    transmittingData.lon = transmittingData.lon / 10000000;  // Convert to float.
    transmittingData.alt = transmittingData.alt / 1000;      // Convert to meters.
  }
#endif

#ifdef DEVMODE
  SerialUSB.print("Lat: ");
  SerialUSB.print(transmittingData.lat, 9);
  SerialUSB.print(" Lon: ");
  SerialUSB.print(transmittingData.lon, 9);
  SerialUSB.print(" Date/Time: ");
  SerialUSB.print(gps.getYear());
  SerialUSB.print("-");
  SerialUSB.print(gps.getMonth());
  SerialUSB.print("-");
  SerialUSB.print(gps.getDay());
  SerialUSB.print(" ");
  SerialUSB.print(gps.getHour());
  SerialUSB.print(":");
  SerialUSB.print(gps.getMinute());
  SerialUSB.print(":");
  SerialUSB.println(gps.getSecond());
  SerialUSB.println("Entering GPS low power mode.");
#endif

  // powerOff uses the 8-byte version of RXM-PMREQ - supported by older (M8) modules.
  //gps.powerOff(sleepForSecs * 1000);

  // powerOffWithInterrupt uses the 16-byte version of RXM-PMREQ - supported by the M10 etc.
  // powerOffWithInterrupt allows us to set the force flag. The M10 integration manual states:
  // "The "force" flag must be set in UBX-RXM-PMREQ to enter software standby mode."
  gps.powerOffWithInterrupt(30000, VAL_RXM_PMREQ_WAKEUPSOURCE_EXTINT0, true);  // No (additional) wakeup sources. force = true

#ifdef DEVMODE
  SerialUSB.print("Transmitting with a packet size of: ");
  SerialUSB.println(sizeof(transmittingData));
#endif
  long beforeTransmit = millis();
  transmit(transmittingData);  // This is blocking and takes a tad bit more than 6.75 seconds. Make this non-blocking if you wish to here: https://github.com/sandeepmistry/arduino-LoRa/blob/7c2ebfd0d839582309c43eb464be84b563459da9/examples/LoRaSenderNonBlocking/LoRaSenderNonBlocking.ino#L10.
  long afterTransmit = millis();
#ifdef DEVMODE
  SerialUSB.print("Done transmitting. Time took: ");
  SerialUSB.print(afterTransmit - beforeTransmit);
  SerialUSB.println(" milliseconds. Now sleeping LoRa and sleeping MCU.");
#endif
  LoRa.sleep();  // The LoRa module wakes up automatically when the SPI interface is active.
  LowPower.deepSleep(sleepTime);
  delay(250);   // Let the proccesor wake up a bit.
  gpsWakeup();  // Wakeup GPS to get a quick fix.
  SerialUSB.println("GPS is awake. MCU is going back to sleep.");
  LowPower.deepSleep(1000);
}
