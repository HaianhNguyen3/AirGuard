# AirGuard — Indoor Air-Quality Monitor

AirGuard is an ESP32-based prototype for monitoring indoor PM2.5 concentration and communicating air-quality status through local visual, audible, and browser-based feedback. The firmware reads a ZH03B particulate sensor, classifies the reading into four configurable states, drives a 12-pixel NeoPixel indicator and a buzzer, and serves a lightweight dashboard over Wi-Fi.

> **Project status:** Prototype / ongoing development. This repository documents the current implementation, not a certified air-quality instrument. Thresholds require validation, and some planned features are not enabled yet.

## Features

- **PM2.5 sensing:** Reads ZH03B data over UART and checks the sensor frame checksum before accepting a reading.
- **Four-state classification:** `good`, `moderate`, `poor`, and `bad`, using configurable thresholds in `config.h`.
- **Physical alerts:** A 12-pixel RGB NeoPixel strip displays the current status; a buzzer changes its alert cadence for poor and bad readings.
- **Local web dashboard:** Displays PM2.5, status, recommendations, and a browser-side trend chart.
- **Live data transport:** Sends JSON updates to connected clients over a WebSocket endpoint (`/ws`).
- **Recent-history endpoint:** Exposes up to 60 PM2.5 points stored in RAM through `GET /history`.
- **Firmware update route:** Includes a browser-based OTA upload route at `/update`; use only on a trusted local network and change the credentials before enabling it.
- **Extensible sensor interface:** Includes an SHT30 temperature/humidity driver, but it is disabled by default and is not considered an integrated feature in the current prototype.

## Hardware

The current pin assignments are defined in `config.h`.

| Component | ESP32 connection | Notes |
|---|---|---|
| ZH03B TX | GPIO 4 (ESP32 RX) | UART, 9600 baud |
| ZH03B RX | GPIO 16 (ESP32 TX) | UART; confirm whether the sensor requires this connection for the selected operating mode |
| NeoPixel DIN | GPIO 5 | 12 RGB LEDs |
| Buzzer signal | GPIO 27 | Check voltage/current requirements of the specific buzzer module |
| SHT30 SDA | GPIO 21 | Planned; disabled by default |
| SHT30 SCL | GPIO 22 | Planned; disabled by default |

Use a suitable common ground and verify the exact pinout and voltage requirements of every module before powering the circuit. Do not power a high-current LED strip directly from an ESP32 GPIO.

## Software requirements

- Arduino IDE with the ESP32 board package installed
- Board target: **ESP32 Dev Module** (adjust if your board differs)
- Libraries:
  - Adafruit NeoPixel
  - ESPAsyncWebServer
  - AsyncTCP (ESP32-compatible version)
  - ArduinoJson 6.x
  - LittleFS support from the ESP32 Arduino core

## Setup

### 1. Configure local credentials

Open `config.h` and replace the placeholders:

```cpp
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define OTA_USERNAME "CHANGE_THIS_USERNAME"
#define OTA_PASSWORD "CHANGE_THIS_TO_A_STRONG_PASSWORD"
#define OTA_ENABLED false  // Keep false until OTA is configured securely.
```

Do not commit real Wi-Fi credentials or production passwords to a public repository. OTA authentication uses HTTP Basic Authentication over plain HTTP; it is not suitable for exposure to the public internet. Disable the OTA route or use a more secure update architecture for deployment beyond a trusted local network.

### 2. Open and build the sketch

1. Open `AirGuard.ino` from this repository in Arduino IDE.
2. Select the matching ESP32 board and serial port.
3. Install the required libraries listed above.
4. Check the pin assignments and threshold values in `config.h`.
5. Compile and upload the sketch over USB.

### 3. Upload the dashboard files to LittleFS

The web server serves static files from the LittleFS path `/web/`. The repository already places the assets in `data/web/`:

```text
data/
└── web/
    ├── index.html
    ├── style.css
    └── app.js
```

Use a compatible ESP32 LittleFS data-upload workflow to upload the `data/` directory to the board's LittleFS partition. Make sure the selected partition scheme has enough filesystem space. Then open the IP address printed in Serial Monitor from a device on the same network.

### 4. Optional OTA update

After the first USB upload, OTA firmware updates may be available at:

```text
http://<ESP32-IP>/update
```

The selected partition scheme must support OTA partitions. Set `OTA_ENABLED` to `true` in `config.h` only after changing the placeholder credentials and selecting an OTA-capable partition scheme. Keep this endpoint on a trusted network only.

## How it works

```text
ZH03B sensor
    │ UART
    ▼
ESP32 firmware ──► PM2.5 classification ──► NeoPixel + buzzer
    │
    ├── WebSocket /ws ──► Browser dashboard
    └── GET /history ───► Recent RAM history (JSON)
```

Current default intervals:

| Operation | Interval |
|---|---:|
| Sensor polling | 3 seconds |
| WebSocket broadcast | 2 seconds |
| RAM history logging | 60 seconds |
| Maximum RAM history | 60 points |

The browser chart keeps its own rolling window of up to 60 received PM2.5 readings. This is separate from the firmware's 60-point RAM history.

## Current limitations and next steps

- **Threshold validation:** The default PM2.5 thresholds are starting values and should be reviewed against the intended standard and validated with controlled tests.
- **No sensor smoothing yet:** The current `processPM25()` function is a placeholder; the latest valid sensor value is used directly.
- **SHT30 not integrated:** Temperature and humidity support is prepared in code but disabled by default; the current firmware should not be presented as measuring these values unless the sensor is connected and the feature is tested.
- **Volatile history:** The firmware history is stored in RAM and is lost when power is removed. Persistent storage or cloud synchronization is not implemented.
- **No automatic air purifier control:** Recommendations are displayed, but the firmware does not control a filtration device.
- **Prototype, not certified:** The device has not been established as a calibrated or regulatory-grade PM2.5 monitor.
- **Testing:** Hardware-in-the-loop tests and measured performance results should be added before reporting accuracy, reliability, or health-impact claims.

## Repository layout

```text
AirGuard/
├── AirGuard.ino
├── config.h
├── AirQuality.h
├── ZH03B.h / ZH03B.cpp
├── SHT30.h / SHT30.cpp
├── NeoPixel.h / NeoPixel.cpp
├── Buzzer.h / Buzzer.cpp
├── OTAManager.h / OTAManager.cpp
├── data.h / data.cpp
├── data/web/
│   ├── index.html
│   ├── style.css
│   └── app.js
├── docs/
│   ├── PROJECT_OVERVIEW.md
│   └── VALIDATION_CHECKLIST.md
└── README.md
```

## Academic and portfolio use

This repository is intended to make the project's architecture, implementation, constraints, and future work transparent to reviewers. Describe only features that have been implemented and tested, and include photos, test methods, and measured results only when they are genuine and reproducible.
