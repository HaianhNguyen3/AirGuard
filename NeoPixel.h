#ifndef AG_NEOPIXEL_H
#define AG_NEOPIXEL_H

#include <Adafruit_NeoPixel.h>
#include "AirQuality.h"

// Vòng NeoPixel 12 LED RGB - cảnh báo bằng màu sắc (mục 5.4 của báo cáo)
class NeoPixelAlarm {
public:
  NeoPixelAlarm(uint8_t pin, uint16_t count, uint8_t brightness);

  void begin();

  // Cập nhật màu LED theo trạng thái chất lượng không khí hiện tại.
  void setState(AirQualityStatus status);
  void update();

private:
  Adafruit_NeoPixel _strip;
  static const uint8_t HISTORY_SIZE = 11;
  AirQualityStatus _history[HISTORY_SIZE];
  uint8_t _historyCount = 0;
  bool _alarmBlinkOn = true;
  unsigned long _lastBlink = 0;

  uint32_t colorForStatus(AirQualityStatus status) const;
  void testLeds();
  void drawHistory();
  void rotateAnimation();
};

#endif
