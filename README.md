# kfs-esp

ESP8266/ESP32 sensor firmware for the KFS home server ecosystem.

Each sensor type has its own directory with a dedicated `.ino` sketch.
Shared infrastructure (WiFi setup, web server, config) lives in `shared/kfs_esp_common.h`.

## Features

- **WiFiManager captive portal** — no hardcoded WiFi credentials. On first boot, the ESP creates an access point `KFS-ESP-Setup`. Connect from your phone, enter WiFi credentials and device name.
- **HTTP server** — serves a human-readable status page on `/` and a JSON API on `/kfsEsp` for automated polling.
- **LittleFS config** — device name persists across reboots.
- **ESP8266 + ESP32** — single codebase, platform selected at compile time.

## Sensor Types

| Directory | Sensor | Measures |
|-----------|--------|----------|
| `kfs_esp_dht22/` | DHT22 (AM2302) | Temperature, Humidity |

## JSON API

`GET /kfsEsp` returns:

```json
{
  "name": "kfs-teplota-office",
  "type": "dht22",
  "data": {
    "temperature": 23.5,
    "humidity": 48.2
  },
  "uptime": 3600
}
```

## Wiring (DHT22 on D1 Mini)

```
D1 Mini          DHT22
--------         -----
3V3        -->   VCC (pin 1)
D5 (GPIO14) -->  DATA (pin 2)
                 NC   (pin 3)
GND        -->   GND  (pin 4)

Pull-up 4.7kΩ between DATA and VCC
(not needed if using a DHT22 breakout board with built-in pull-up)
```

## Build & Flash

### Arduino CLI

```bash
# Install board support
arduino-cli core install esp8266:esp8266
# or for ESP32:
arduino-cli core install esp32:esp32

# Install libraries
arduino-cli lib install "WiFiManager" "DHT sensor library" "Adafruit Unified Sensor"

# Compile (ESP8266, 1MB flash, 64KB LittleFS)
arduino-cli compile \
  --fqbn esp8266:esp8266:d1_mini:eesz=1M64 \
  kfs_esp_dht22/

# Flash
arduino-cli upload \
  --fqbn esp8266:esp8266:d1_mini \
  --port /dev/cu.usbserial-* \
  kfs_esp_dht22/
```

### First Boot

1. ESP creates WiFi AP `KFS-ESP-Setup`
2. Connect from phone/laptop
3. Captive portal opens automatically
4. Enter WiFi SSID, password, and device name (e.g., `kfs-teplota-office`)
5. ESP saves config to flash and connects to WiFi
6. Navigate to ESP's IP to see the status page

### Reset WiFi Config

To clear saved WiFi credentials and re-enter the captive portal, erase the flash:

```bash
python3 -m esptool --port /dev/cu.usbserial-* erase_flash
```

Then re-flash the firmware.

## Adding a New Sensor Type

1. Create a new directory, e.g., `kfs_esp_ds18b20/`
2. Create `kfs_esp_ds18b20.ino` with:
   - `#define SENSOR_TYPE "ds18b20"` before the include
   - `#include "../shared/kfs_esp_common.h"`
   - Implement `kfsReadSensor()`, `kfsDataJson()`, `kfsDataHtml()`
3. Compile and flash

## License

MIT
