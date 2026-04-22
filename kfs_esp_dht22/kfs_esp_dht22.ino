/*
 * kfs_esp_dht22 — KFS ESP sensor firmware for DHT22 (AM2302)
 *
 * Measures temperature and humidity every 10 seconds.
 * Serves data via HTTP: human-readable page on / and JSON API on /kfsEsp.
 * WiFi configured via captive portal on first boot.
 *
 * Wiring:
 *   DHT22 DATA → GPIO14 (D5 on D1 Mini / GPIO14 on ESP-12F)
 *   DHT22 VCC  → 3.3V
 *   DHT22 GND  → GND
 *   (pull-up resistor 4.7kΩ on DATA line if not on breakout board)
 */

#define SENSOR_TYPE "dht22"

#include "../shared/kfs_esp_common.h"
#include <DHT.h>

// --- Hardware config ---
#define DHT_PIN   14
#define DHT_TYPE  DHT22

DHT dht(DHT_PIN, DHT_TYPE);

// --- Sensor data ---
static float sensorTemp = NAN;
static float sensorHum  = NAN;
static bool  sensorOk   = false;

// --- Callbacks required by kfs_esp_common.h ---

void kfsReadSensor() {
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (isnan(t) || isnan(h)) {
        Serial.println("DHT read error");
        sensorOk = false;
        return;
    }

    sensorTemp = t;
    sensorHum  = h;
    sensorOk   = true;
    Serial.printf("%.1f C  %.1f %%\n", t, h);
}

String kfsDataJson() {
    if (!sensorOk) {
        return "\"temperature\":null,\"humidity\":null";
    }
    return "\"temperature\":" + String(sensorTemp, 1) +
           ",\"humidity\":" + String(sensorHum, 1);
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
           "<div class='label'>Humidity</div>"
           "<div class='value'>" + String(sensorHum, 1) + " %</div>"
           "</div>";
}

// --- Arduino entry points ---

void setup() {
    kfsSetup();
    dht.begin();
    Serial.println("DHT22 sensor initialized on GPIO" + String(DHT_PIN));
}

void loop() {
    kfsLoop();
}
