#pragma once

// Enables.
#define DEVMODE    // Comment this out when you're ready to do the actual flight to save even more power.
#define TEST_COORD // Uncomment this to use test coordinates instead of actual coordinates.

// Pins.
#define DIO0_PIN 2         // Arduino pin that connects to LoRa module.
#define RESET_PIN 5        // Arduino pin that connects to LoRa module.
#define WAKEUP_PIN 8       // Pin to wakeup GPS module from software backup mode.
#define LED 13             // Pin for indicator LED of the tracker
#define BAT_VOLTAGE_PIN A0 // Pin for battery voltage measurement.
#define SS_PIN 38          // Arduino pin that connects to LoRa module. AKA CS.

// Radio settings.
#define SYNC_WORD 0xFB      // Only other devices with this sync word can receive your broadcast.
#define FREQUENCY 433E6     // Frequency of your LoRa module.
#define SPREADING_FACTOR 10 // Choose this based on this https://forum.arduino.cc/t/what-are-the-best-settings-for-a-lora-radio/449528.
#define BANDWIDTH 62.5E3    // Choose this based on this https://forum.arduino.cc/t/what-are-the-best-settings-for-a-lora-radio/449528.

// Others.
#define COMMENT "XXXXXX"     // Put your callsign or text payload here.
#define COMMENT_SIZE 7       // Size of the comment. Find number of characters and add one.
#define BAUD_RATE 115200     // SerialUSB baud rate.
const int sleepTime = 10000; // How many ms do you want to sleep for between packets?
