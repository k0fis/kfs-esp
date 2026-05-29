/*
 * kfs_esp_bme280 — KFS ESP sensor firmware for BME280/BMP280
 *
 * Measures temperature, humidity (BME only), and pressure every 10 seconds.
 * Serves data via HTTP: human-readable page on / and JSON API on /kfsEsp.
 *
 * Wiring (I2C on D1 Mini):
 *   SDA → D2 (GPIO4)
 *   SCL → D1 (GPIO5)
 *   VCC → 5V (module has onboard regulator)
 *   GND → GND
 *
 * I2C address: 0x77 (default) or 0x76
 */

#define SENSOR_TYPE "bme280"

#include <kfs_esp_common.h>
#include <Wire.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;

// --- Sensor data ---
static float sensorTemp     = NAN;
static float sensorHum      = NAN;
static float sensorPressure = NAN;
static bool  sensorOk       = false;
static bool  hasHumidity    = false;

// --- Callbacks required by kfs_esp_common.h ---

void kfsReadSensor() {
    float t = bme.readTemperature();
    float p = bme.readPressure() / 100.0F;  // Pa → hPa
    float h = bme.readHumidity();

    if (!isnan(t) && t > -40 && t < 85) {
        sensorTemp = t;
        sensorPressure = p;
        sensorOk = true;

        if (!isnan(h) && h > 0 && h <= 100) {
            sensorHum = h;
            hasHumidity = true;
        }

        Serial.printf("%.1f C  %.1f hPa", t, p);
        if (hasHumidity) Serial.printf("  %.1f %%", h);
        Serial.println();
    } else {
        sensorOk = false;
        Serial.println("BME280 read error");
    }
}

String kfsDataJson() {
    if (!sensorOk) {
        return "\"temperature\":null,\"humidity\":null,\"pressure\":null";
    }
    String json = "\"temperature\":" + String(sensorTemp, 1) +
                  ",\"pressure\":" + String(sensorPressure, 1);
    if (hasHumidity) {
        json += ",\"humidity\":" + String(sensorHum, 1);
    } else {
        json += ",\"humidity\":null";
    }
    return json;
}

String kfsDataHtml() {
    if (!sensorOk) {
        return "<div class='card'>"
               "<div class='label'>Sensor</div>"
               "<div class='value' style='color:#ef4444'>No data</div>"
               "</div>";
    }
    String html = "<div class='card'>"
                  "<div class='label'>Temperature</div>"
                  "<div class='value'>" + String(sensorTemp, 1) + " &deg;C</div>"
                  "</div>"
                  "<div class='card'>"
                  "<div class='label'>Pressure</div>"
                  "<div class='value'>" + String(sensorPressure, 1) + " hPa</div>"
                  "</div>";
    if (hasHumidity) {
        html += "<div class='card'>"
                "<div class='label'>Humidity</div>"
                "<div class='value'>" + String(sensorHum, 1) + " %</div>"
                "</div>";
    }
    return html;
}

// --- Arduino entry points ---

void setup() {
    Wire.begin(4, 5);  // SDA=GPIO4(D2), SCL=GPIO5(D1)
    kfsSetup();
    delay(100);

    if (!bme.begin(0x77, &Wire)) {
        delay(100);
        if (!bme.begin(0x76, &Wire)) {
            Serial.println("BME280 not found! Check wiring.");
            return;
        }
    }
    Serial.println("BME280 initialized on I2C");

    // First read to detect if humidity is available (BME vs BMP)
    kfsReadSensor();
    if (hasHumidity) {
        Serial.println("Humidity detected → BME280");
    } else {
        Serial.println("No humidity → BMP280 (temp+pressure only)");
    }
}

void loop() {
    kfsLoop();
}
