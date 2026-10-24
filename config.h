#ifndef AIRGUARD_CONFIG_H
#define AIRGUARD_CONFIG_H

// ==================== WI-FI ====================
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// ==================== ZH03B (UART) ====================
#define ZH03B_RX_PIN 4  // ESP32 RX <- ZH03B TX
#define ZH03B_TX_PIN 16 // ESP32 TX -> ZH03B RX
#define ZH03B_BAUD 9600

// ==================== SHT30 (I2C) ====================
// Chưa tích hợp trong phiên bản hiện tại (xem mục 11 - Hạn chế của báo cáo).
// Định nghĩa được giữ sẵn để phục vụ mục 12.1 - Bổ sung SHT30.
#define SHT30_I2C_ADDR 0x44
#define SHT30_SDA_PIN 21
#define SHT30_SCL_PIN 22
#define SHT30_ENABLED false // đổi thành true khi đã gắn cảm biến thật

// ==================== NeoPixel ====================
#define NEOPIXEL_PIN 5
#define NEOPIXEL_COUNT 12
#define NEOPIXEL_BRIGHTNESS 60 // 0-255

// ==================== Buzzer ====================
#define BUZZER_PIN 27

// ==================== Ngưỡng phân loại PM2.5 (µg/m³) ====================
// Thiết kế dạng tham số để dễ hiệu chỉnh mà không cần thay đổi phần cứng
// (mục 6.3 của báo cáo). Đây là giá trị mặc định, có thể tinh chỉnh sau.
#define PM25_THRESHOLD_GOOD     12.0f   // < 12   : Tốt
#define PM25_THRESHOLD_MODERATE 35.0f   // 12-35  : Trung bình
#define PM25_THRESHOLD_POOR     55.0f   // 35-55  : Kém
                                         // > 55   : Xấu

// ==================== Nhịp đọc / gửi dữ liệu ====================
#define SENSOR_READ_INTERVAL_MS 3000    // đọc cảm biến mỗi 3s
#define WEBSOCKET_SEND_INTERVAL_MS 2000 // gửi dữ liệu mỗi 2s
#define DATA_LOG_INTERVAL_MS 60000      // lưu 1 điểm dữ liệu mỗi 60s (cho biểu đồ)

// ==================== Lưu trữ dữ liệu (mục 12.2) ====================
#define DATA_HISTORY_SIZE 60 // số điểm lưu trong RAM (vd: 60 điểm gần nhất)

// ==================== Web OTA ====================
// Đổi 2 giá trị này trước khi dùng thật - nếu để mặc định, bất kỳ ai trong
// cùng mạng Wi-Fi cũng có thể ghi đè firmware của thiết bị.
#define OTA_ENABLED false  // Enable only after setting credentials and an OTA-capable partition scheme.
#define OTA_USERNAME "CHANGE_THIS_USERNAME"
#define OTA_PASSWORD "CHANGE_THIS_TO_A_STRONG_PASSWORD"

#endif
