#ifndef AG_DATA_H
#define AG_DATA_H

#include <Arduino.h>
#include "config.h"

// Một điểm dữ liệu lịch sử PM2.5 (mục 12.2 - Lưu trữ dữ liệu của báo cáo).
// Hiện tại lưu trong RAM (vòng đệm - ring buffer); phiên bản sau có thể
// thay bằng SPIFFS/LittleFS hoặc gửi lên server/cloud (mục 12.5).
struct DataPoint {
  unsigned long timestamp;  // millis() tại thời điểm ghi
  float pm25;
};

class DataStore {
public:
  DataStore();

  // Thêm một điểm dữ liệu mới vào vòng đệm.
  void addPoint(float pm25);

  size_t count() const { return _count; }
  const DataPoint &at(size_t i) const;  // i = 0 là điểm cũ nhất còn lưu

  float average() const;
  float maxValue() const;

private:
  DataPoint _buffer[DATA_HISTORY_SIZE];
  size_t _head = 0;   // vị trí ghi tiếp theo
  size_t _count = 0;  // số điểm đã ghi (<= DATA_HISTORY_SIZE)
};

#endif
