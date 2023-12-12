#define comment "XXXXXX"
#define baudRate 115200     // SerialUSB baud rate.
#define syncWord 0xFB       // Only other devices with this sync word can receive your broadcast.
#define ssPin 38            // Arduino pin that connects to LoRa module. AKA csPin.
#define resetPin 5          // Arduino pin that connects to LoRa module.
#define DIO0Pin 2           // Arduino pin that connects to LoRa module.
#define frequency 433E6     // Frequency of your LoRa module.
#define spreadingFactor 12  // Choose this based on this https://forum.arduino.cc/t/what-are-the-best-settings-for-a-lora-radio/449528.
#define bandwidth 62.5E3    // Choose this based on this https://forum.arduino.cc/t/what-are-the-best-settings-for-a-lora-radio/449528.
#define wakeupPin 8         // Pin to wakeup GPS module from software backup mode.
// #define devMode             // Comment this out when you're ready to do the actual flight to save even more power.
// #define testCoord           // Uncomment this to use test coordinates instead of actual coordinates.
#define sleepTime 10000      // How many ms do you want to sleep for between packets?
