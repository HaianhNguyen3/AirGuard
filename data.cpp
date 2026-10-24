#include "data.h"

DataStore::DataStore() {}

void DataStore::addPoint(float pm25) {
  _buffer[_head] = { millis(), pm25 };
  _head = (_head + 1) % DATA_HISTORY_SIZE;
  if (_count < DATA_HISTORY_SIZE) {
    _count++;
  }
}

const DataPoint &DataStore::at(size_t i) const {
  // i=0 la diem cu nhat con luu trong vong dem
  size_t start = (_count < DATA_HISTORY_SIZE) ? 0 : _head;
  size_t idx = (start + i) % DATA_HISTORY_SIZE;
  return _buffer[idx];
}

float DataStore::average() const {
  if (_count == 0) return 0;
  float sum = 0;
  for (size_t i = 0; i < _count; i++) {
    sum += at(i).pm25;
  }
  return sum / _count;
}

float DataStore::maxValue() const {
  if (_count == 0) return 0;
  float m = at(0).pm25;
  for (size_t i = 1; i < _count; i++) {
    if (at(i).pm25 > m) m = at(i).pm25;
  }
  return m;
}
