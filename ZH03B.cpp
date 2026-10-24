#include "ZH03B.h"

ZH03B::ZH03B(HardwareSerial &serial, int rxPin, int txPin)
  : _serial(serial), _rxPin(rxPin), _txPin(txPin) {}

void ZH03B::begin(uint32_t baud) {
  _serial.begin(baud, SERIAL_8N1, _rxPin, _txPin);
}

bool ZH03B::update() {
  // Gom byte cho tới khi tìm được header 0x42 0x4D hợp lệ, sau đó đọc đủ khung.
  while (_serial.available() >= FRAME_LEN) {
    if (_serial.peek() != 0x42) {
      _serial.read();  // bỏ byte rác, dịch cửa sổ tìm header
      continue;
    }
    if (!readFrame()) {
      return false;
    }
    if (!verifyChecksum()) {
      continue;  // khung lỗi, thử lại ở vòng lặp tiếp theo
    }
    // Byte 10-11: PM2.5 nồng độ chuẩn khí quyển (atmospheric), đơn vị µg/m³
    uint16_t pm25Raw = (_buf[10] << 8) | _buf[11];
    _pm25 = (float)pm25Raw;
    _ready = true;
    return true;
  }
  return false;
}

bool ZH03B::readFrame() {
  size_t n = _serial.readBytes(_buf, FRAME_LEN);
  return n == FRAME_LEN && _buf[0] == 0x42 && _buf[1] == 0x4D;
}

bool ZH03B::verifyChecksum() const {
  uint16_t sum = 0;
  for (uint8_t i = 0; i < FRAME_LEN - 2; i++) {
    sum += _buf[i];
  }
  uint16_t checksum = (_buf[FRAME_LEN - 2] << 8) | _buf[FRAME_LEN - 1];
  return sum == checksum;
}
