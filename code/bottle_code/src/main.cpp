#include <Arduino.h>
#include <Wire.h>
#include "i2cscanner.h"
#include <Arduino_LSM6DSOX.h>

#include "fx29.h"
#include "FX29K.h"

FX29K scale(FX29K0, 0010, &Wire);

#define SDA_PIN 7
#define SCL_PIN 6

// FX29 loadCell;
I2CScanner scanner;
void setup() {
  Serial.begin(115200);
  
  
  
  unsigned long serialStart = millis();
  while (!Serial && millis() - serialStart < 3000) {
    delay(10);
  }

  Wire.begin(SDA_PIN, SCL_PIN);
  delay(2000);
  log_e("Debug logging is now enabled!");

  Serial.println("Booting...");
  Serial.print("I2C pins SDA=");
  Serial.print(SDA_PIN);
  Serial.print(" SCL=");
  Serial.println(SCL_PIN);

  // Scan I2C bus to see what's actually connected
  Serial.println("Scanning I2C bus...");
  scanner.printDiscoverableDevices();

  if (!IMU.begin()) {
    Serial.println("Failed to initialize LSM6DSOX!");
    while (1) {
      delay(1000);
    }
  }
  scale.tare(1000);

  

  // if (loadCell.isConnected()) {
  //   Serial.println("FX29 connected.");
  // } else {
  //   Serial.println("FX29 not found on I2C bus.");
  // }
}


void loop() {
  // float lb;
  // loadCell.readForce(lb);
  // Serial.print("Lbs: ");
  // Serial.println(lb);
  uint16_t raw = scale.getRawBridgeData();
  float g = scale.getGrams();
  float lb = scale.getPounds();
  Serial.print(g, 1);
  Serial.print("\t");
  Serial.print(lb, 3);
  Serial.print("\t");
  Serial.println(raw);

  float x, y, z;

  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);
    Serial.print("Accel: ");
    Serial.print(x); Serial.print('\t');
    Serial.print(y); Serial.print('\t');
    Serial.println(z);
  }

  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(x, y, z);
    Serial.print("Gyro:  ");
    Serial.print(x); Serial.print('\t');
    Serial.print(y); Serial.print('\t');
    Serial.println(z);
  }
  delay(1000);
}
