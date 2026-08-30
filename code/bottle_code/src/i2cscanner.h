#ifndef I2C_SCANNER_H
#define I2C_SCANNER_H

#include <Arduino.h>
#include <Wire.h>

class I2CScanner {
public:
    // Constructor
    I2CScanner();

    

    // Scans the bus, prints found devices to Serial, and returns the count
    int printDiscoverableDevices();

    // Checks if a specific address is responsive
    bool isDevicePresent(byte address);
    
};

#endif // I2C_SCANNER_H
