// LittleLoRa.
// Charles Nicholson, 2023.
// Code for a LoRa GPS tracker.
// Meant to be used to track a high-altitude glider.
// https://github.com/crnicholson/littleLoRa/.

// ***** NOTE *****
// This system operates at 433 MHz, so a ham radio license is needed in the US.
// Please check local regulations before use. Please also input your callsign
// in the settings.h file.

// ***** GPS *****
// This tracker takes advantage of the special abilities of the u-blox MAX-M10S
// module. If you choose to modify this code to use a different GPS, a
// different GPS library will need to be used. Furthermore, it is extremely
// likely you will lose the ability to enter power saving mode.

// ***** GPS Low Power *****
// There are two pins of the module you can use to wakeup the module:
// VAL_RXM_PMREQ_WAKEUPSOURCE_UARTRX  = uartrx
// VAL_RXM_PMREQ_WAKEUPSOURCE_EXTINT0 = extint0 (default)
// These values can be or'd (|) together to enable interrupts on multiple pins.
// On the SAMD side, a digital pin is needed that is capable of driving HIGH
// and LOW. In this case, we use the digital pin 8 of the SAMD. More
// information on the configuration of the power save mode can be found in
// "Example22_PowerOff" of the Sparkfun GNSS v3 library.

// ***** SAMD21G18A *****
// This tracker uses a SAMD21G18A microcontroller because of its many benefits.
// The SAMD needs a bootloader burned onto it when you get it from Digikey. My
// friend is working on a tutorial for burning a bootloader. The bootloader
// burns as an Arduino Zero, native USB port. The pin definitions are for the
// Arduino Zero, which is essentially the same as an UNO. If you wish not to
// use the SAMD, your life will become much harder. Trust me. Just use it.
// Forget about the ancient ATMega328.

// ***** SAMD21G18A Low Power *****
// The SAMD goes to sleep to save power. This is implemented using the
// ArduinoLowPower library. With the LoRa, MCU, and u-blox in sleep, plus the
// MCP1700 voltage regulator, the current draw can be as low as 280 uA while
// retaining function. To achieve proper sleep, some edits to the SAMD core are
// necessary. To find the wiring.c file on your computer, follow this guide:
// https://support.arduino.cc/hc/en-us/articles/4415103213714-Find-sketches-libraries-board-cores-and-other-files-on-your-computer.
// Once there, comment out this line as shown:
/*
  // Setup all pins (digital and analog) in INPUT mode (default is nothing)
  for (uint32_t ul = 0 ; ul < NUM_DIGITAL_PINS ; ul++ )
  {
    pinMode( ul, INPUT ) ;
  }
*/
// Arduino gives you an official warning, but from my testing, it doesn't
// effect anything: "This breaks Arduino APIs since all pins are considered
// INPUT at startup. However, it really lowers the power consumption by a
// factor of 20 in low power mode (0.03mA vs 0.6mA)"

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

// For finding LoRa link budget and transmit time, follow this link:
// https://www.semtech.com/design-support/lora-calculator.
// Above link also includes other important LoRa factors.

// If you don't want to use LoRa for some odd reason, there are other options.
// One option is to use my friend's tracker, Tiny4FSK. Link here:
// https://github.com/New-England-Weather-Balloon-Society/Tiny4FSK/tree/main

// ***** To-Do *****
// - Documentation
// - Add a temperature sensor
// - Acquire and transmit speed and course
// - Make an app for the receiver

#include "headers/settings.h"
#include <ArduinoLowPower.h>
#include <LoRa.h>                    // LoRa.
#include <SPI.h>                     // Needed for LoRa.
#include <SparkFun_u-blox_GNSS_v3.h> // http://librarymanager/All#SparkFun_u-blox_GNSS_v3.
#include <Wire.h>                    // Needed for I2C to GPS module.

struct __attribute__((packed)) dataStruct {
  float lat;
  float lon;
  short volts;
  short alt; // These are longs to make keep the data types the same between the transmitter and the receiver. Converting to ints is getting worked on to make the payload shorter.
  byte sats;
  byte speed;
  short course;
  byte seconds;
  byte minutes;
  byte hours;
  short txCount;
  char text[COMMENT_SIZE] = COMMENT;
} payload;

int fixType = 0;
int day = 0;
int month = 0;
int year = 0;

#ifdef TEST_COORD
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

  pinMode(BAT_VOLTAGE_PIN, INPUT);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  longPulse(); // Pulse LED to show that power is supplied. 

#ifdef DEVMODE
  SerialUSB.begin(BAUD_RATE);
  while (!SerialUSB)
    ; // Wait until SerialUSB is all good.
  SerialUSB.println("LittleLoRa Tracker.");
#endif

  LoRa.setPins(SS_PIN, RESET_PIN, DIO0_PIN); // Has to be before LoRa.begin().

  if (!LoRa.begin(FREQUENCY)) {
#ifdef DEVMODE
    SerialUSB.println("Starting LoRa failed!");
#endif
    while (1)
      ;
  }

  LoRa.setSyncWord(SYNC_WORD);               // Defined in settings.h.
  LoRa.setSpreadingFactor(SPREADING_FACTOR); // Defined in settings.h.
  LoRa.setSignalBandwidth(BANDWIDTH);        // Defined in settings.h.
  LoRa.crc();                                // Get a checksum.

  Wire.begin();

  if (gps.begin() == false) { // Connect to the u-blox module using Wire port.
#ifdef DEVMODE
    SerialUSB.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
#endif
    while (1)
      ;
  }

  // gps.hardReset(); // Hard reset - force a cold start.

#ifndef TEST_COORD
  while ((fixType < 3) && (payload.sats < 5)) { // Make sure location is valid before the first sleep.
    if (gps.getPVT()) {
      getData();
#ifdef DEVMODE
      displayData();
#endif
    } else {
#ifdef DEVMODE
      SerialUSB.println("No PVT data received. Retrying...");
#endif
    }
  }
#endif

  // Following commented out code allows for use of PSMOO (or PSMCT with some edits).
  // PSMOO allows for predefined periodic wakeup of the GPS receiver to get a fix, then go into backup mode.
  // PSMCT allows for the GPS to get a fix, then go into Power Efficient Tracking (POT) state. This is good for trackers that transmit a lot.
  // To use either PSMCT or PSMOO, you can remove the wait-for-fix code and the software backup part. Uncomment the lines below, too.
  // You may need to do more editing of the code to achieve your desired outcome, as this has not been tested yet.
  /*
    gpsConfig();

    uint8_t PSM;

    if (gps.getVal8(UBLOX_CFG_PM_OPERATEMODE, &PSM) ==
        true) { // Test if the GPS config worked correctly.
  #ifdef DEVMODE
      if (PSM == 1) {
        SerialUSB.println("Power save mode set correctly!");
      } else {
        SerialUSB.println("Power save mode configuration failed!");
      }
    } else {
      SerialUSB.println("VALGET failed!");
    }
  */
}

void loop() {
  shortPulse();
  readVoltage();
#ifndef TEST_COORD
  payload.sats = 0;
  fixType = 0;
  while ((fixType < 3) && (payload.sats < 5)) { // Make sure location is valid from wakeup before transmitting location.
    if (gps.getPVT()) {
      getData();
#ifdef DEVMODE
      displayData();
#endif
    } else {
#ifdef DEVMODE
      SerialUSB.println("No PVT data received. Retrying...");
#endif
    }
  }
#endif

  payload.txCount++;

#ifdef TEST_COORD
  payload.lat = lat;
  payload.lon = lon;
  payload.alt = alt;
  payload.sats = sats;
  payload.speed = speed;
  payload.course = course;
  displayData();
#endif

#ifdef DEVMODE
  SerialUSB.println("Entering GPS low power mode.");
#endif

  // powerOff uses the 8-byte version of RXM-PMREQ - supported by older (M8) modules, like so:
  // gps.powerOff(sleepForSecs * 1000);

  // powerOffWithInterrupt uses the 16-byte version of RXM-PMREQ - supported by the M10 etc.
  // powerOffWithInterrupt allows us to set the force flag.
  // The M10 integration manual states: "The "force" flag must be set in UBX-RXM-PMREQ to enter software standby mode."
  gps.powerOffWithInterrupt(sleepTime * 3, VAL_RXM_PMREQ_WAKEUPSOURCE_EXTINT0, true); // No (additional) wakeup sources. force = true.

#ifdef DEVMODE
  SerialUSB.print("Transmitting with a packet size of: ");
  SerialUSB.println(sizeof(payload));
#endif
  long beforeTransmit = millis();
  transmit(payload); // This is blocking and takes a tad bit more than a second. Make this non-blocking if you wish to here: https://github.com/sandeepmistry/arduino-LoRa/blob/7c2ebfd0d839582309c43eb464be84b563459da9/examples/LoRaSenderNonBlocking/LoRaSenderNonBlocking.ino#L10.
  long afterTransmit = millis();
#ifdef DEVMODE
  SerialUSB.print("Done transmitting. Time took: ");
  SerialUSB.print(afterTransmit - beforeTransmit);
  SerialUSB.println(" milliseconds. Now sleeping LoRa and sleeping MCU.");
#endif
  LoRa.sleep(); // The LoRa module wakes up automatically when the SPI interface is active.
  LowPower.deepSleep(sleepTime);
  gpsWakeup(); // Wakeup GPS.
#ifdef DEVMODE
  SerialUSB.println("GPS and MCU are awake.");
#endif
}