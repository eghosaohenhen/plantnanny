// #include <Arduino.h>
// #include <Wire.h>
// #include <i2cscanner.h>

// // global variables // const float maxForce = 10.0; // 10 lbf capacity
// const int FX29_EXPECTED_ADDR = 0x28; // Default FX29 I2C address

// // put function declarations here:
// int myFunction(int, int);

// void setup() {
//   Wire.begin();        // Join I2C Bus
//   Serial.begin(9600);  // Start serial monitor
//   delay(2000);

//   Serial.println("Initializing system...");

//   I2CScanner scanner;



  
//   // Call the imported function
//   // 1. Print all visible devices
//     int count = scanner.printDiscoverableDevices();

//     // 2. Target verification check
//     if (scanner.isDevicePresent(FX29_EXPECTED_ADDR)) {
//         Serial.println("Target FX29 Load Cell is online!");
//     } else {
//         Serial.println("Warning: Target FX29 Load Cell is missing from the bus.");
//         while (true) {
//           Serial.println("nothing");
//           delay(1000); 
//         }
//     }
// }

// void loop() {
//   Wire.requestFrom(FX29_EXPECTED_ADDR, 2); // Request 2 bytes from the load cell
  
//   if (Wire.available() == 2) {
//     byte byteHigh = Wire.read();
//     byte byteLow = Wire.read();
    
//     // Combine bytes into a single 14-bit raw value
//     // (Clearing the top 2 status bits if used by the bridge)
//     int rawCounts = ((byteHigh & 0x3F) << 8) | byteLow; 
    
//     // Convert counts to Force (lbf) using the factory 1,000 to 15,000 span
//     float force = 0.0;
//     if (rawCounts > 1000) {
//       force = ((float)(rawCounts - 1000) / 14000.0) * maxForce;
//     }

//     Serial.print("Raw Counts: ");
//     Serial.print(rawCounts);
//     Serial.print(" | Force: ");
//     Serial.print(force);
//     Serial.println(" lbf");
//   } else {
//     Serial.println("Sensor not responding on I2C bus.");
//   }

//   delay(500); // Poll every half second
// }


// // put function definitions here:
// int myFunction(int x, int y) {
//   return x + y;
// }
