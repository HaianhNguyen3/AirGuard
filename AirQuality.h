#ifndef AIRGUARD_AIRQUALITY_H
#define AIRGUARD_AIRQUALITY_H

#include <Arduino.h>
#include "config.h"

// Các mức chất lượng không khí (mục 6.3 của báo cáo)
enum AirQualityStatus {
  AQ_UNKNOWN = -1,
  AQ_GOOD = 0,       // Tốt
  AQ_MODERATE = 1,   // Trung bình
  AQ_POOR = 2,       // Kém
  AQ_BAD = 3         // Xấu
};

// Phân loại PM2.5 -> trạng thái chất lượng không khí.
// Ngưỡng được định nghĩa dạng tham số trong config.h để dễ hiệu chỉnh
// mà không cần thay đổi phần cứng.
inline AirQualityStatus classifyPM25(float pm25) {
  if (pm25 < PM25_THRESHOLD_GOOD)      return AQ_GOOD;
  if (pm25 < PM25_THRESHOLD_MODERATE)  return AQ_MODERATE;
  if (pm25 < PM25_THRESHOLD_POOR)      return AQ_POOR;
  return AQ_BAD;
}

// Nhãn hiển thị (dùng cho JSON / Web / log)
inline const char *statusLabel(AirQualityStatus status) {
  switch (status) {
    case AQ_GOOD:     return "good";
    case AQ_MODERATE: return "moderate";
    case AQ_POOR:     return "poor";
    case AQ_BAD:      return "bad";
    default:          return "unknown";
  }
}

// Khuyến nghị tương ứng với từng trạng thái (mục 8 của báo cáo)
inline const char *recommendationFor(AirQualityStatus status) {
  switch (status) {
    case AQ_GOOD:     return "Không cần thực hiện hành động";
    case AQ_MODERATE: return "Tiếp tục theo dõi";
    case AQ_POOR:     return "Hạn chế nguồn phát sinh bụi";
    case AQ_BAD:      return "Đóng cửa, bật hệ thống lọc khí";
    default:          return "Đang chờ dữ liệu cảm biến";
  }
}

#endif
