#define BAUD_RATE 115200    // Serial baud rate.
#define SYNC_WORD 0xFB      // Only other devices with this sync word can receive your broadcast.
#define SS_PIN 38           // Arduino pin that connects to LoRa module. AKA CS.
#define RESET_PIN 5         // Arduino pin that connects to LoRa module.
#define DIO0_PIN 2          // Arduino pin that connects to LoRa module.
#define FREQUENCY 433E6     // Frequency of your LoRa module.
#define SPREADING_FACTOR 10 // Choose this based on this https://forum.arduino.cc/t/what-are-the-best-settings-for-a-lora-radio/449528.
#define BANDWIDTH 62.5E3    // Choose this based on this https://forum.arduino.cc/t/what-are-the-best-settings-for-a-lora-radio/449528.
#define DEVMODE             // Comment this out when you're ready to do the actual flight to save even more power.