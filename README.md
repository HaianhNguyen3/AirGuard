# AirGuard – Code hoàn chỉnh (dựa trên báo cáo)

Bộ code này hiện thực hoá đúng cấu trúc chương trình mô tả trong **Phụ lục A**
của báo cáo (`AirGuard.ino`, `sensors/`, `web/`, `alarm/`, `storage/`), cùng
với các chức năng đã mô tả trong các mục 5–9 và 12.

## 1. Thư viện cần cài (Arduino IDE → Library Manager)
- `Adafruit NeoPixel`
- `ESPAsyncWebServer`
- `AsyncTCP` (bản cho ESP32)
- `ArduinoJson` (v6.x)
- `LittleFS` (đi kèm sẵn trong ESP32 core)

Board: **ESP32 Dev Module** (khớp với ESP32 DevKit 30 pin ở mục 10.1).

## 2. Đấu nối phần cứng (mặc định trong `config.h`)
| Linh kiện        | Chân ESP32 |
|-------------------|-----------|
| ZH03B TX → RX2     | GPIO16    |
| ZH03B RX ← TX2     | GPIO17    |
| NeoPixel DIN       | GPIO4     |
| Buzzer             | GPIO27    |
| SHT30 SDA (dự kiến)| GPIO21    |
| SHT30 SCL (dự kiến)| GPIO22    |

Có thể đổi chân trực tiếp trong `config.h`.

## 3. Cấu hình Wi-Fi và ngưỡng cảnh báo
Sửa trong `config.h`:
- `WIFI_SSID`, `WIFI_PASSWORD`
- `PM25_THRESHOLD_GOOD / MODERATE / POOR` – ngưỡng phân loại (mục 6.3),
  thiết kế dạng tham số để dễ hiệu chỉnh mà không cần đổi phần cứng.
- `SHT30_ENABLED` – để `false` vì SHT30 **chưa được tích hợp** trong phiên
  bản hiện tại (mục 11 – Hạn chế). Khi gắn cảm biến thật, đổi thành `true`.

## 4. Upload giao diện Web lên ESP32 (LittleFS)
Thư mục `web/` chứa `index.html`, `style.css`, `app.js`. Trình phục vụ web
đọc các file này từ đường dẫn `/web/` trên LittleFS, vì vậy trước khi upload
bằng công cụ *ESP32 LittleFS Data Upload*:

1. Tạo một thư mục tên **`data`** ngay trong thư mục sketch (`AirGuard/data/`).
2. Copy toàn bộ thư mục `web/` vào trong đó, để có `AirGuard/data/web/index.html`, v.v.
3. Dùng công cụ "ESP32 Sketch Data Upload" (Arduino IDE) để ghi vào LittleFS.
4. Upload chương trình chính (`AirGuard.ino`) như bình thường.

## 5. Cách hoạt động
- `setup()` gọi lần lượt: `initSensors → initLED → initBuzzer → initWiFi → initWebServer`.
- `loop()` gọi lần lượt: `readSensors → processPM25 → updateAirQuality → updateAlarm
  → sendWebSocketData → saveData`, đúng theo lưu đồ ở mục A.2 của báo cáo.
- Dữ liệu được đóng gói JSON và gửi qua WebSocket tới trình duyệt theo đúng
  định dạng ví dụ trong mục 6.2:
  ```json
  {"pm25":28.5,"temperature":29.2,"humidity":68,
   "status":"moderate","recommendation":"Tiếp tục theo dõi"}
  ```
- Màu LED theo đúng bảng ở mục 5.4: Tốt=Xanh lá, Trung bình=Vàng, Kém=Cam, Xấu=Đỏ.
- Khuyến nghị theo đúng bảng ở mục 8.
- `storage/data.*` lưu vòng đệm (ring buffer) `DATA_HISTORY_SIZE` điểm PM2.5
  gần nhất trong RAM, phục vụ vẽ biểu đồ xu hướng (mục 12.2) qua endpoint
  `GET /history`. Đây là bước đệm cho hướng phát triển "lưu trữ dữ liệu";
  có thể thay bằng ghi file LittleFS hoặc gửi lên cloud sau này (mục 12.5).

## 6. Cập nhật firmware qua OTA (Web OTA)

Đã tích hợp sẵn `ota/OTAManager` — dùng chung `AsyncWebServer` hiện có, không
cần cài thêm thư viện (chỉ dùng `Update.h` có sẵn trong ESP32 core).

**Bước 1 — bắt buộc: đổi Partition Scheme.**
Arduino IDE → Tools → Partition Scheme → chọn một scheme có OTA (ví dụ
"Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)"). Nếu không đổi, ESP32
sẽ không có đủ 2 phân vùng `ota_0`/`ota_1` để ghi firmware mới.

**Bước 2 — đổi tài khoản trong `config.h`:**
```cpp
#define OTA_USERNAME "admin"
#define OTA_PASSWORD "mat-khau-cua-ban"
```

**Bước 3 — nạp firmware lần đầu bằng USB như bình thường** (lần đầu bắt
buộc phải qua USB; từ lần sau có thể dùng OTA).

**Bước 4 — build ra file `.bin` cho các lần cập nhật sau:**
Arduino IDE → Sketch → Export Compiled Binary. File `.bin` sẽ nằm trong
thư mục sketch (ví dụ `AirGuard.ino.bin`).

**Bước 5 — cập nhật qua trình duyệt:**
1. Mở `http://<địa-chỉ-IP-cua-ESP32>/update`
2. Đăng nhập bằng `OTA_USERNAME` / `OTA_PASSWORD`
3. Chọn file `.bin`, bấm "Tai len va cap nhat"
4. Thiết bị tự khởi động lại và chạy firmware mới.

**Hoặc cập nhật bằng dòng lệnh** (tiện khi viết script tự động hoá sau này):
```bash
curl -u admin:mat-khau-cua-ban \
     -F "firmware=@AirGuard.ino.bin" \
     http://<dia-chi-IP-cua-ESP32>/update
```
`-F "firmware=@file"` gửi file dạng `multipart/form-data`, đúng với tên
field `firmware` mà `OTAManager` đang lắng nghe.

## 7. Hạn chế đã biết (khớp mục 11 của báo cáo)
- SHT30 có sẵn class điều khiển nhưng **mặc định tắt**, chưa gắn phần cứng thật.
- Ngưỡng cảnh báo là giá trị khởi điểm, cần hiệu chỉnh thực nghiệm.
- Dữ liệu lịch sử lưu trong RAM (mất khi mất điện) – chưa có cơ sở dữ liệu lâu dài.
- Hệ thống lọc bụi chưa được điều khiển trong code này (mới ở mức định hướng).
