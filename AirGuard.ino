/*
 * AirGuard - Thiết bị giám sát và cảnh báo chất lượng không khí trong nhà
 *
 * Cấu trúc chương trình theo Phụ lục A của báo cáo:
 *   setup(): initSensors, initLED, initBuzzer, initWiFi, initWebServer
 *   loop() : readSensors, processPM25, updateAirQuality, updateAlarm,
 *            sendWebSocketData, saveData
 *
 * Thư viện cần cài (Arduino Library Manager):
 *   - Adafruit NeoPixel
 *   - ESPAsyncWebServer  (https://github.com/mathieucarbou/ESPAsyncWebServer)
 *   - AsyncTCP           (https://github.com/mathieucarbou/AsyncTCP)
 *   - ArduinoJson (v6+)
 *   - LittleFS (đi kèm ESP32 core, dùng để phục vụ file trong thư mục web/)
 *
 * Board: ESP32 DevKit 30 pin (mục 10.1 của báo cáo)
 */

#include <WiFi.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#include "config.h"
#include "AirQuality.h"
#include "ZH03B.h"
#include "SHT30.h"
#include "NeoPixel.h"
#include "Buzzer.h"
#include "data.h"
#include "OTAManager.h"

// ==================== Đối tượng toàn cục ====================
ZH03B dustSensor(Serial1, ZH03B_RX_PIN, ZH03B_TX_PIN);
SHT30 envSensor(SHT30_I2C_ADDR);
NeoPixelAlarm ledAlarm(NEOPIXEL_PIN, NEOPIXEL_COUNT, NEOPIXEL_BRIGHTNESS);
BuzzerAlarm buzzerAlarm(BUZZER_PIN);
DataStore history;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// ==================== Trạng thái hệ thống ====================
float currentPM25 = 0;
bool pm25Ready = false;  // Do not report a healthy reading before the sensor returns valid data.
float currentTemperature = NAN;
float currentHumidity = NAN;
AirQualityStatus currentStatus = AQ_UNKNOWN;

unsigned long lastSensorRead = 0;
unsigned long lastWebSocketSend = 0;
unsigned long lastDataLog = 0;

// ==================== setup() - các hàm khởi tạo ====================

void initSensors() {
  dustSensor.begin(ZH03B_BAUD);

  if (SHT30_ENABLED) {
    envSensor.begin(SHT30_SDA_PIN, SHT30_SCL_PIN);
  }

  Serial.println("[initSensors] ZH03B da san sang."
                  " SHT30: " + String(SHT30_ENABLED ? "bat" : "chua tich hop"));
}

void initLED() {
  ledAlarm.begin();
  Serial.println("[initLED] NeoPixel 12 LED da san sang.");
}

void initBuzzer() {
  buzzerAlarm.begin();
  Serial.println("[initBuzzer] Buzzer da san sang.");
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("[initWiFi] Dang ket noi Wi-Fi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("[initWiFi] Da ket noi. Dia chi IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("[initWiFi] Khong the ket noi Wi-Fi, se thu lai trong loop().");
  }
}

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                       AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("[WebSocket] Client #%u da ket noi\n", client->id());
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("[WebSocket] Client #%u da ngat ket noi\n", client->id());
  }
}

void initWebServer() {
  if (!LittleFS.begin(true)) {
    Serial.println("[initWebServer] Loi khi mount LittleFS.");
  }

  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);

  // Phục vụ giao diện web tĩnh trong thư mục web/ (index.html, style.css, app.js)
  server.serveStatic("/", LittleFS, "/web/").setDefaultFile("index.html");

  // Endpoint lấy lịch sử PM2.5 gần nhất, phục vụ vẽ biểu đồ (mục 12.2)
  server.on("/history", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<2048> doc;
    JsonArray arr = doc.to<JsonArray>();
    for (size_t i = 0; i < history.count(); i++) {
      JsonObject point = arr.createNestedObject();
      point["t"] = history.at(i).timestamp;
      point["pm25"] = history.at(i).pm25;
    }
    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
  });

  // OTA is disabled by default for safer public-repository defaults.
  // Enable OTA_ENABLED only after setting strong credentials and an OTA-capable partition scheme.
  if (OTA_ENABLED) {
    OTAManager::begin(server, OTA_USERNAME, OTA_PASSWORD);
  }

  server.begin();
  Serial.println("[initWebServer] Web Server + WebSocket da san sang.");
  if (OTA_ENABLED) {
    Serial.println("[initWebServer] Cap nhat firmware tai: http://" + WiFi.localIP().toString() + "/update");
  }
}

// ==================== loop() - các hàm xử lý liên tục ====================

void readSensors() {
  // ZH03B cập nhật liên tục qua UART (không chặn - non-blocking)
  if (dustSensor.update()) {
    currentPM25 = dustSensor.getPM25();
    pm25Ready = true;
  }

  if (SHT30_ENABLED) {
    float t, h;
    if (envSensor.read(t, h)) {
      currentTemperature = t;
      currentHumidity = h;
    }
  }
}

void processPM25() {
  // Bước lọc/làm mượt đơn giản có thể bổ sung tại đây (vd: trung bình trượt).
  // Hiện tại sử dụng trực tiếp giá trị tức thời từ ZH03B.
}

void updateAirQuality() {
  if (!pm25Ready) {
    currentStatus = AQ_UNKNOWN;
  } else {
    currentStatus = classifyPM25(currentPM25);
  }
  ledAlarm.setState(currentStatus);
}

void updateAlarm() {
  ledAlarm.update();
  buzzerAlarm.update(currentStatus);  // cần gọi thường xuyên để xử lý nhịp bíp
}

void sendWebSocketData() {
  if (ws.count() == 0) return;  // không có client nào đang xem, bỏ qua để tiết kiệm tài nguyên

  StaticJsonDocument<256> doc;
  if (pm25Ready) {
    doc["pm25"] = currentPM25;
  }
  if (SHT30_ENABLED) {
    doc["temperature"] = currentTemperature;
    doc["humidity"] = currentHumidity;
  }
  doc["status"] = statusLabel(currentStatus);
  doc["recommendation"] = recommendationFor(currentStatus);

  String payload;
  serializeJson(doc, payload);
  ws.textAll(payload);
}

void saveData() {
  if (pm25Ready) {
    history.addPoint(currentPM25);
  }
}

// ==================== setup() ====================

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n=== AirGuard - Khoi dong he thong ===");

  initSensors();
  initLED();
  initBuzzer();
  initWiFi();
  initWebServer();

  Serial.println("=== AirGuard san sang ===\n");
}

// ==================== loop() ====================

void loop() {
  unsigned long now = millis();

  if (now - lastSensorRead >= SENSOR_READ_INTERVAL_MS) {
    lastSensorRead = now;
    readSensors();
    processPM25();
    updateAirQuality();
  }

  updateAlarm();  // gọi mỗi vòng lặp để buzzer nhấp nháy đúng nhịp

  if (now - lastWebSocketSend >= WEBSOCKET_SEND_INTERVAL_MS) {
    lastWebSocketSend = now;
    sendWebSocketData();
  }

  if (now - lastDataLog >= DATA_LOG_INTERVAL_MS) {
    lastDataLog = now;
    saveData();
  }

  // Wi-Fi tự động thử kết nối lại nếu bị rớt
  if (WiFi.status() != WL_CONNECTED) {
    static unsigned long lastReconnectAttempt = 0;
    if (now - lastReconnectAttempt > 10000) {
      lastReconnectAttempt = now;
      WiFi.reconnect();
    }
  }
}
