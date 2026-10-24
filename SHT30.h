#ifndef SHT30_H
#define SHT30_H

#include <Arduino.h>
#include <Wire.h>

// Cảm biến nhiệt độ / độ ẩm SHT30.
// Theo mục 11 (Hạn chế) của báo cáo, SHT30 CHƯA được tích hợp vào phiên bản
// hiện tại. Class này cung cấp sẵn giao diện + cài đặt I2C cơ bản để việc
// bổ sung SHT30 (mục 12.1) sau này chỉ cần bật SHT30_ENABLED trong config.h.
class SHT30 {
public:
  SHT30(uint8_t i2cAddr = 0x44);

  bool begin(int sdaPin, int sclPin);

  // Đọc nhiệt độ (°C) và độ ẩm (%RH). Trả về true nếu đọc thành công.
  bool read(float &temperature, float &humidity);

private:
  uint8_t _addr;
  bool _initialized = false;
};

#endif
