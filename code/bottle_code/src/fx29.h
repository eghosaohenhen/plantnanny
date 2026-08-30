#ifndef FX29_H
#define FX29_H
#define ADDRESS 0x28
#define MAXFORCELB 10.0f

#include <Arduino.h>
#include <Wire.h>

class FX29 {
public:
    FX29();

    bool isConnected();
    bool readMeasurement();
    bool readForce(float &x);

private:
    uint8_t address;
    float maxForceLb;
};

#endif
