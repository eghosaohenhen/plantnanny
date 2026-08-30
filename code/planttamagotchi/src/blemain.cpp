// /*
//  * BLE scanner for XIAO ESP32-C3 — prints every nearby device's MAC + name.
//  * Use it to find your water bottle's real BLE address for the connect sketch.
//  *
//  * Library: NimBLE-Arduino by h2zero (this targets 2.x — the current version).
//  *   PlatformIO:  lib_deps = h2zero/NimBLE-Arduino
//  *   Arduino IDE: Library Manager -> "NimBLE-Arduino"
//  * If you're on NimBLE 1.x, see the notes at the bottom for the 2 changes.
//  *
//  * Open Serial Monitor at 115200. Drink from / press the bottle so it
//  * advertises, and watch for a line flagged "<-- possible bottle?".
//  */
// #include <Arduino.h>
// #include <NimBLEDevice.h>
// #include <set>
// #include <string>

// // Names get flagged if they contain any of these (case-insensitive).
// const char* HINTS[] = { "boost", "water", "bottle", "hydr", "sip", "drink" };

// std::set<std::string> seen;   // de-dupe so each device prints once

// static bool looksLikeBottle(std::string name) {
//   for (auto& c : name) c = tolower(c);
//   for (const char* h : HINTS)
//     if (name.find(h) != std::string::npos) return true;
//   return false;
// }

// static void printHex(const std::string& data) {
//   for (size_t i = 0; i < data.length(); i++)
//     Serial.printf("%02X ", (uint8_t)data[i]);
// }

// // Only print devices at least this strong — hold the bottle ON the board.
// // Raise toward -30 if still too noisy; lower toward -70 if the bottle is faint.
// const int RSSI_MIN = -55;

// class ScanCallbacks : public NimBLEScanCallbacks {
//   void onResult(const NimBLEAdvertisedDevice* dev) override {
//     int rssi = dev->getRSSI();
//     if (rssi < RSSI_MIN) return;          // too far away — ignore the clutter

//     // Skip Apple gear (iPhone/Watch/AirPods/Mac): mfg data starts with 4C 00.
//     std::string md = dev->getManufacturerData();
//     if (md.length() >= 2 && (uint8_t)md[0] == 0x4C && (uint8_t)md[1] == 0x00) return;

//     std::string name = dev->getName();
//     std::string mac  = dev->getAddress().toString();

//     // De-dupe on NAME when we have one (MACs rotate for privacy); else on MAC.
//     std::string key = name.empty() ? mac : name;
//     if (seen.count(key)) return;
//     seen.insert(key);

//     Serial.println("--------------------------------------------------");
//     Serial.printf("MAC:  %s\n", mac.c_str());
//     Serial.printf("Name: %s%s\n",
//                   name.empty() ? "(none)" : name.c_str(),
//                   (!name.empty() && looksLikeBottle(name)) ? "   <-- possible bottle?" : "");
//     Serial.printf("RSSI: %d dBm\n", rssi);

//     // Advertised service UUIDs — note any custom (128-bit) ones.
//     if (dev->getServiceUUIDCount() > 0) {
//       Serial.print("Services: ");
//       for (int i = 0; i < dev->getServiceUUIDCount(); i++)
//         Serial.printf("%s  ", dev->getServiceUUID(i).toString().c_str());
//       Serial.println();
//     }

//     // Manufacturer data — the bottle may hide an intake counter here.
//     // (md was already fetched above for the Apple filter — reuse it.)
//     if (!md.empty()) {
//       Serial.print("MfgData: ");
//       printHex(md);
//       Serial.println();
//     }
//   }
// };

// void setup() {
//   Serial.begin(115200);
//   delay(3000);
//   Serial.println("\nBLE scanner starting...");

//   NimBLEDevice::init("");
//   NimBLEScan* pScan = NimBLEDevice::getScan();
//   pScan->setScanCallbacks(new ScanCallbacks());
//   pScan->setActiveScan(true);   // request scan-response so we get names
//   pScan->setInterval(100);
//   pScan->setWindow(100);
//   pScan->start(0);              // 0 = scan forever
//   Serial.println("Scanning (reset the board to clear the list and rescan)...");
// }

// void loop() {
//   delay(1000);
// }

// /*
//  * NimBLE 1.x differences, if it won't compile:
//  *   1. Base class is  NimBLEAdvertisedDeviceCallbacks  (not NimBLEScanCallbacks),
//  *      and onResult takes a non-const pointer:
//  *          void onResult(NimBLEAdvertisedDevice* dev) override
//  *   2. Register it with  pScan->setAdvertisedDeviceCallbacks(new ScanCallbacks());
//  *      (not setScanCallbacks)
//  */
