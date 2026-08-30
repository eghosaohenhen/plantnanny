#include "fx29.h"

FX29::FX29() {
    address = ADDRESS; 
    maxForceLb = MAXFORCELB;
}
bool FX29::readForce(float &lb){
  uint8_t bytes[2];
  uint16_t data[1];
  Wire.requestFrom(address, (uint8_t)1);   // Read_MR: trigger a measurement (discard byte)
  Wire.read();
  delay(10);
  // Step 3: Data Fetch — read the 2 result bytes
  if (Wire.requestFrom(address, (uint8_t)2) != 2) {
    Serial.println("Sensor not responding on I2C bus.");
    return false;
  }
  bytes[0] = Wire.read();
  bytes[1] = Wire.read();
  uint8_t status = (bytes[0] >> 6) & 0x03; // lowk what is the point of the mask part 
  data[0] = ((bytes[0] & 0x3F) << 8) | bytes[1];
  log_w("FX29 raw bytes from Wire.read 1st: %x 2nd: %x", bytes[0], bytes[1]);
  log_w("FX29 data & status: %x  %x", data[0], status);

  lb = (data[0] - 1638.0f) * maxForceLb / (14746.0f - 1638.0f);
  return true;
}


bool FX29::readMeasurement() {
  // Step 1: Send Measurement Request (MR)
  Wire.beginTransmission(address);
  Wire.endTransmission();
  
  // Step 2: Wait for measurement to complete (datasheet says ~3ms)
  delay(10);
  
  // Step 3: Now actually read the 2 bytes
  Wire.requestFrom(address, (uint8_t)2);
  
  if (Wire.available() == 2) {
    byte byteHigh = Wire.read();
    byte byteLow = Wire.read();
    
    // Check status bits (top 2 bits of byteHigh)
    byte status = (byteHigh >> 6) & 0x03;
    if (status != 0) {
      Serial.print("Sensor status error: ");
      Serial.println(status);
      return false;
    }
    
    // Combine into 14-bit raw value
    Serial.print("Bytes: ");
    Serial.print(byteHigh, HEX);
    Serial.print(' ');
    Serial.print(byteLow, HEX);
    Serial.print(" | ");
    int rawCounts = ((byteHigh & 0x3F) << 8) | byteLow;
    
    float force = 0.0;
    if (rawCounts > 1000) {
      force = ((float)(rawCounts - 1000) / 14000.0) * maxForceLb;
    }

    Serial.print("Raw Counts: ");
    Serial.print(rawCounts);
    Serial.print(" | Force: ");
    Serial.print(force);
    Serial.println(" lbf");
  } else {
    Serial.println("Sensor not responding on I2C bus.");
    return false;
  }

  return true;
}

bool FX29::isConnected() {
  Wire.beginTransmission(address);
  return (Wire.endTransmission() == 0);
}
