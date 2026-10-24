#ifndef ZH03B_H
#define ZH03B_H

#include <Arduino.h>

// Cảm biến bụi ZH03B - chế độ Initiative Upload (UART, khung 24 byte)
// Tham khảo mục 5.2 của báo cáo: PM2.5 là thông số trung tâm của hệ thống.
class ZH03B {
public:
  ZH03B(HardwareSerial &serial, int rxPin, int txPin);

  void begin(uint32_t baud = 9600);

  // Đọc và giải mã một khung dữ liệu nếu có sẵn trên UART.
  // Trả về true nếu đọc được một giá trị PM2.5 mới hợp lệ.
  bool update();

  // Giá trị PM2.5 (µg/m³) gần nhất đọc được, hợp lệ sau khi update() trả về true.
  float getPM25() const { return _pm25; }

  // Cờ báo cảm biến đã từng đọc được dữ liệu hợp lệ hay chưa.
  bool isReady() const { return _ready; }

private:
  HardwareSerial &_serial;
  int _rxPin;
  int _txPin;
  float _pm25 = 0;
  bool _ready = false;

  static const uint8_t FRAME_LEN = 24;
  uint8_t _buf[FRAME_LEN];

  bool readFrame();
  bool verifyChecksum() const;
};

#endif
