#include "NeoPixel.h"

NeoPixelAlarm::NeoPixelAlarm(uint8_t pin, uint16_t count, uint8_t brightness)
  : _strip(count, pin, NEO_GRB + NEO_KHZ800) {
  _strip.setBrightness(brightness);
}

void NeoPixelAlarm::begin() {
  _strip.begin();
  testLeds();
  _strip.clear();
  _strip.show();
}

void NeoPixelAlarm::testLeds() {
  const uint8_t brightnessLevels[] = {10, 60, 255};
  const uint32_t colors[] = {
    _strip.Color(255, 0, 0),     // Red
    _strip.Color(0, 255, 0),     // Green
    _strip.Color(0, 0, 255),     // Blue
    _strip.Color(255, 255, 0),   // Yellow
    _strip.Color(0, 255, 255),   // Cyan
    _strip.Color(255, 0, 255),   // Magenta
    _strip.Color(255, 255, 255)  // White
  };

  for (uint8_t brightness : brightnessLevels) {
    _strip.setBrightness(brightness);

    for (uint32_t color : colors) {
      for (uint16_t i = 0; i < _strip.numPixels(); i++) {
        _strip.setPixelColor(i, color);
      }
      _strip.show();
      delay(250);
    }
  }
}

void NeoPixelAlarm::setState(AirQualityStatus status) {
  rotateAnimation();

  // Add the newest reading at LED 12 and shift older readings clockwise.
  if (_historyCount < HISTORY_SIZE) {
    _historyCount++;
  }

  for (uint8_t i = _historyCount - 1; i > 0; i--) {
    _history[i] = _history[i - 1];
  }
  _history[0] = status;

  drawHistory();
}

void NeoPixelAlarm::update() {
  bool hasAlarm = false;
  for (uint8_t i = 0; i < _historyCount; i++) {
    if (_history[i] == AQ_POOR || _history[i] == AQ_BAD) {
      hasAlarm = true;
      break;
    }
  }

  if (!hasAlarm) return;

  unsigned long now = millis();
  if (now - _lastBlink >= 500) {
    _lastBlink = now;
    _alarmBlinkOn = !_alarmBlinkOn;
    drawHistory();
  }
}

void NeoPixelAlarm::drawHistory() {
  _strip.clear();
  for (uint8_t i = 0; i < _historyCount; i++) {
    AirQualityStatus status = _history[i];
    bool alarming = status == AQ_POOR || status == AQ_BAD;
    if (alarming && !_alarmBlinkOn) continue;
    _strip.setPixelColor(HISTORY_SIZE - 1 - i, colorForStatus(status));
  }
  _strip.show();
}

void NeoPixelAlarm::rotateAnimation() {
  const uint32_t rotationColor = _strip.Color(255, 255, 255);

  for (uint8_t step = 0; step < _strip.numPixels(); step++) {
    _strip.clear();
    _strip.setPixelColor(step, rotationColor);
    _strip.show();
    delay(35);
  }
}

uint32_t NeoPixelAlarm::colorForStatus(AirQualityStatus status) const {
  // Bảng màu theo mục 5.4 của báo cáo
  switch (status) {
    case AQ_GOOD:     return _strip.Color(0, 255, 0);    // Xanh lá - Không khí tốt
    case AQ_MODERATE: return _strip.Color(255, 255, 0);  // Vàng    - Cần theo dõi
    case AQ_POOR:     return _strip.Color(255, 120, 0);  // Cam     - Ô nhiễm
    case AQ_BAD:      return _strip.Color(255, 0, 0);    // Đỏ      - Ô nhiễm cao
    default:           return _strip.Color(0, 0, 40);    // Xanh mờ - chưa có dữ liệu
  }
}
