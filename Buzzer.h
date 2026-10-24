#ifndef AG_BUZZER_H
#define AG_BUZZER_H

#include <Arduino.h>
#include "AirQuality.h"

// Buzzer 3.3V - cảnh báo âm thanh (mục 5.5 của báo cáo)
// - Không cảnh báo (Tốt/Trung bình): tắt
// - Cảnh báo nhẹ (Kém): phát âm thanh ngắt quãng
// - Cảnh báo cao (Xấu): tăng tần suất cảnh báo
class BuzzerAlarm {
public:
  explicit BuzzerAlarm(uint8_t pin);

  void begin();

  // Cần được gọi thường xuyên trong loop() để xử lý nhịp bíp ngắt quãng.
  void update(AirQualityStatus status);

private:
  uint8_t _pin;
  unsigned long _lastToggle = 0;
  bool _on = false;

  unsigned long intervalForStatus(AirQualityStatus status) const;
};

#endif
