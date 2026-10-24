#include "OTAManager.h"
#include <Update.h>

// Lưu tạm username/password để 2 lambda callback bên dưới dùng chung
// (AsyncWebServer yêu cầu callback dạng hàm/lambda không thành viên).
static const char *_otaUser = nullptr;
static const char *_otaPass = nullptr;

// Trang HTML tối giản để test upload file .bin bằng tay.
// PROGMEM: lưu chuỗi này trong bộ nhớ flash thay vì RAM (RAM trên ESP32 rất hạn chế).
static const char UPLOAD_FORM_HTML[] PROGMEM = R"====(
<!DOCTYPE html><html lang="vi"><head><meta charset="UTF-8">
<title>AirGuard - Cap nhat firmware</title></head>
<body style="font-family:sans-serif;max-width:420px;margin:60px auto;">
<h2>Cap nhat firmware AirGuard</h2>
<form method="POST" action="/update" enctype="multipart/form-data">
  <input type="file" name="firmware" accept=".bin" required><br><br>
  <button type="submit">Tai len va cap nhat</button>
</form>
<p style="color:#888;font-size:13px;">Thiet bi se tu khoi dong lai sau khi cap nhat xong.</p>
</body></html>
)====";

void OTAManager::begin(AsyncWebServer &server, const char *username, const char *password) {
  _otaUser = username;
  _otaPass = password;

  // ---- GET /update: hiển thị form upload ----
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
    // request->authenticate(): kiểm tra HTTP Basic Auth (username/password
    // gõ trong hộp thoại đăng nhập của trình duyệt).
    if (_otaUser && _otaPass && !request->authenticate(_otaUser, _otaPass)) {
      return request->requestAuthentication();  // trả về 401, trình duyệt tự hiện popup đăng nhập
    }
    request->send_P(200, "text/html", UPLOAD_FORM_HTML);
  });

  // ---- POST /update: nhận file .bin ----
  // AsyncWebServer tách route POST kèm file-upload thành 2 callback:
  //   1) "onRequest"  - chạy 1 LẦN, SAU KHI toàn bộ file đã nhận xong -> trả response
  //   2) "onUpload"   - chạy NHIỀU LẦN, mỗi lần nhận 1 đoạn (chunk) dữ liệu file
  server.on(
    "/update",
    HTTP_POST,
    // --- (1) onRequest: gọi sau khi upload xong ---
    [](AsyncWebServerRequest *request) {
      bool ok = !Update.hasError();

      AsyncWebServerResponse *response = request->beginResponse(
        200,
        "text/plain",
        ok ? "OK, dang khoi dong lai..." : "That bai, kiem tra lai file .bin"
      );
      response->addHeader("Connection", "close");
      request->send(response);

      if (ok) {
        // delay ngắn để chắc chắn response đã gửi xong tới trình duyệt
        // trước khi ESP32 tự khởi động lại (nếu restart ngay, client có
        // thể chưa kịp nhận phản hồi và tưởng là lỗi).
        delay(500);
        ESP.restart();
      }
    },
    // --- (2) onUpload: gọi liên tục trong lúc file đang được tải lên ---
    [](AsyncWebServerRequest *request, String filename, size_t index,
       uint8_t *data, size_t len, bool final) {

      if (_otaUser && _otaPass && !request->authenticate(_otaUser, _otaPass)) {
        return request->requestAuthentication();
      }

      // index == 0: đây là chunk ĐẦU TIÊN của file -> khởi tạo quá trình ghi OTA
      if (index == 0) {
        Serial.printf("[OTA] Bat dau nhan file: %s\n", filename.c_str());

        // UPDATE_SIZE_UNKNOWN: không biết trước tổng dung lượng file
        // (AsyncWebServer không luôn cung cấp Content-Length khi upload
        // dạng multipart), Update sẽ tự tính khi kết thúc.
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
          Update.printError(Serial);   // in lỗi chi tiết ra Serial Monitor để debug
        }
      }

      // Ghi trực tiếp đoạn dữ liệu (data, len) vào phân vùng OTA đang chờ.
      // Update.write() trả về SỐ BYTE đã ghi thành công - so sánh với len
      // để phát hiện ghi thiếu (vd: flash đầy, lỗi phần cứng).
      if (Update.write(data, len) != len) {
        Update.printError(Serial);
      }

      // final == true: đây là chunk CUỐI CÙNG -> hoàn tất quá trình ghi
      if (final) {
        // Update.end(true): tham số true nghĩa là "đặt kích thước firmware
        // bằng đúng số byte đã ghi được" - đồng thời kiểm tra checksum nội
        // bộ để xác nhận file không bị hỏng trong quá trình truyền.
        if (Update.end(true)) {
          Serial.printf("[OTA] Ghi thanh cong %u byte. Cho khoi dong lai...\n",
                         (unsigned)(index + len));
        } else {
          Update.printError(Serial);
        }
      }
    }
  );

  Serial.println("[OTA] Web OTA san sang tai http://<dia-chi-IP>/update");
}
