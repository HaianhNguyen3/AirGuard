#ifndef AG_OTA_MANAGER_H
#define AG_OTA_MANAGER_H

#include <ESPAsyncWebServer.h>

// Web OTA - cho phép cập nhật firmware bằng cách tải file .bin lên qua
// trình duyệt (route GET/POST "/update"), dùng chung AsyncWebServer đã có
// sẵn trong AirGuard thay vì mở thêm cổng/server riêng.
class OTAManager {
public:
  // Gắn 2 route "/update" (GET: hiển thị form, POST: nhận file) vào server
  // đã tồn tại. Gọi hàm này TRƯỚC server.begin() trong initWebServer().
  static void begin(AsyncWebServer &server, const char *username, const char *password);
};

#endif
