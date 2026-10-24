#include "Buzzer.h"

BuzzerAlarm::BuzzerAlarm(uint8_t pin) : _pin(pin) {}

void BuzzerAlarm::begin() {
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
}

unsigned long BuzzerAlarm::intervalForStatus(AirQualityStatus status) const {
  switch (status) {
    case AQ_POOR: return 800;  // cảnh báo nhẹ: nhịp bíp chậm
    case AQ_BAD:  return 250;  // cảnh báo cao: nhịp bíp nhanh, tần suất cao
    default:      return 0;    // Tốt / Trung bình: không cảnh báo
  }
}

void BuzzerAlarm::update(AirQualityStatus status) {
  unsigned long interval = intervalForStatus(status);

  if (interval == 0) {
    if (_on) {
      digitalWrite(_pin, LOW);
      _on = false;
    }
    return;
  }

  unsigned long now = millis();
  if (now - _lastToggle >= interval) {
    _lastToggle = now;
    _on = !_on;
    digitalWrite(_pin, _on ? HIGH : LOW);
  }
}
