/*
 * kfs_esp_bmp180 — KFS ESP sensor firmware for BMP180/BMP085
 *
 * Measures temperature and barometric pressure every 10 seconds.
 * Serves data via HTTP: human-readable page on / and JSON API on /kfsEsp.
 *
 * Wiring (I2C on D1 Mini):
 *   SDA → D2 (GPIO4)
 *   SCL → D1 (GPIO5)
 *   VCC → 5V (module has onboard regulator)
 *   GND → GND
 *
 * I2C address: 0x77 (fixed)
 */

#define SENSOR_TYPE "bmp180"

#include <kfs_esp_common.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>

Adafruit_BMP085 bmp;
static bool sensorInitialized = false;

#define ALTITUDE_M 233.0

// --- Sensor data ---
static float sensorTemp     = NAN;
static float sensorPressure = NAN;
static bool  sensorOk       = false;

// --- Callbacks required by kfs_esp_common.h ---

void kfsReadSensor() {
    if (!sensorInitialized) return;

    float t = bmp.readTemperature();
    float p = bmp.readSealevelPressure(ALTITUDE_M) / 100.0F;  // Pa → hPa (sea level)

    if (!isnan(t) && t > -40 && t < 85) {
        sensorTemp = t;
        sensorPressure = p;
        sensorOk = true;
        Serial.printf("%.1f C  %.1f hPa\n", t, p);
    } else {
        sensorOk = false;
        Serial.println("BMP180 read error");
    }
}

String kfsDataJson() {
    if (!sensorOk) {
        return "\"temperature\":null,\"pressure\":null";
    }
    return "\"temperature\":" + String(sensorTemp, 1) +
           ",\"pressure\":" + String(sensorPressure, 1);
}

String kfsDataHtml() {
    if (!sensorOk) {
        return "<div class='card'>"
               "<div class='label'>Sensor</div>"
               "<div class='value' style='color:#ef4444'>No data</div>"
               "</div>";
    }
    return "<div class='card'>"
           "<div class='label'>Temperature</div>"
           "<div class='value'>" + String(sensorTemp, 1) + " &deg;C</div>"
           "</div>"
           "<div class='card'>"
           "<div class='label'>Pressure</div>"
           "<div class='value'>" + String(sensorPressure, 1) + " hPa</div>"
           "</div>";
}

// --- Arduino entry points ---

void setup() {
    Wire.begin(4, 5);
    kfsSetup();
    delay(100);

    if (!bmp.begin()) {
        Serial.println("BMP180 not found! Check wiring.");
        return;
    }
    sensorInitialized = true;
    Serial.println("BMP180 initialized on I2C");
    kfsReadSensor();
}

void loop() {
    kfsLoop();
}
