#define COMMENT "XXXXXX"
#define BAUD_RATE 115200     // SerialUSB baud rate.
#define SYNC_WORD 0xFB       // Only other devices with this sync word can receive your broadcast.
#define SS_PIN 38            // Arduino pin that connects to LoRa module. AKA csPin.
#define RST_PIN 5          // Arduino pin that connects to LoRa module.
#define DIO0PIN 2           // Arduino pin that connects to LoRa module.
#define FREQ 433E6     // Frequency of your LoRa module.
#define SPREADING_FACTOR 12  // Choose this based on this https://forum.arduino.cc/t/what-are-the-best-settings-for-a-lora-radio/449528.
#define BANDWIDTH 62.5E3    // Choose this based on this https://forum.arduino.cc/t/what-are-the-best-settings-for-a-lora-radio/449528.
#define WAKEUP_PIN 8         // Pin to wakeup GPS module from software backup mode.
// #define devMode             // Comment this out when you're ready to do the actual flight to save even more power.
// #define testCoord           // Uncomment this to use test coordinates instead of actual coordinates.
#define SLEEP_TIME 10000      // How many ms do you want to sleep for between packets?
