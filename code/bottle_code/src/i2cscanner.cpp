#include "i2cscanner.h"

I2CScanner::I2CScanner() {
    // Constructor
}

int I2CScanner::printDiscoverableDevices() {
    int devicesFound = 0;
    Serial.println("\n--- Initiating Class-Based I2C Scan ---");

    for (byte address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        byte error = Wire.endTransmission();

        if (error == 0) {
            Serial.print("Found device at address: 0x");
            if (address < 16) Serial.print("0");
            Serial.println(address, HEX);
            devicesFound++;
        }
        else if (error == 4) {
            Serial.print("Unknown hardware error at address: 0x");
            if (address < 16) Serial.print("0");
            Serial.println(address, HEX);
        }
    }

    if (devicesFound == 0) {
        Serial.println("No I2C devices responded on the bus.");
    } else {
        Serial.print("Scan complete. Total devices detected: ");
        Serial.println(devicesFound);
    }
    Serial.println("---------------------------------------\n");

    return devicesFound;
}

bool I2CScanner::isDevicePresent(byte address) {
    Wire.beginTransmission(address);
    return (Wire.endTransmission() == 0);
}
