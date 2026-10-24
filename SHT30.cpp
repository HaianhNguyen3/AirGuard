#include "SHT30.h"

SHT30::SHT30(uint8_t i2cAddr) : _addr(i2cAddr) {}

bool SHT30::begin(int sdaPin, int sclPin) {
  Wire.begin(sdaPin, sclPin);
  _initialized = true;
  return true;
}

bool SHT30::read(float &temperature, float &humidity) {
  if (!_initialized) return false;

  // Lệnh đo single-shot, high repeatability (0x2C06)
  Wire.beginTransmission(_addr);
  Wire.write(0x2C);
  Wire.write(0x06);
  if (Wire.endTransmission() != 0) {
    return false;
  }

  delay(15);  // thời gian đo của SHT30

  Wire.requestFrom((int)_addr, 6);
  if (Wire.available() < 6) {
    return false;
  }

  uint8_t data[6];
  for (int i = 0; i < 6; i++) {
    data[i] = Wire.read();
  }

  uint16_t rawTemp = (data[0] << 8) | data[1];
  uint16_t rawHum  = (data[3] << 8) | data[4];

  temperature = -45.0f + 175.0f * ((float)rawTemp / 65535.0f);
  humidity    = 100.0f * ((float)rawHum / 65535.0f);

  return true;
}
