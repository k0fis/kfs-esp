/*
 * kfs_esp_scd41 — KFS ESP sensor firmware for Sensirion SCD41
 *
 * Measures CO2, temperature, and humidity.
 * SCD41 measurement cycle is ~5 seconds — reads every 10s (KFS_SENSOR_INTERVAL).
 * Serves data via HTTP: human-readable page on / and JSON API on /kfsEsp.
 *
 * Hardware: ESP32-C3 Super Mini + LaskaKit SCD41 module
 *
 * Wiring (I2C):
 *   SDA → GPIO8
 *   SCL → GPIO9
 *   VCC → 3.3V
 *   GND → GND
 *   (pull-ups integrated on SCD41 module)
 *
 * I2C address: 0x62 (fixed, cannot be changed)
 */

#define SENSOR_TYPE "scd41"

#include <kfs_esp_common.h>
#include <Wire.h>
#include <SensirionI2cScd4x.h>

// --- Hardware config ---
#define SCD41_SDA  8
#define SCD41_SCL  9

SensirionI2cScd4x scd4x;

// --- Sensor data ---
static uint16_t sensorCo2  = 0;
static float    sensorTemp = NAN;
static float    sensorHum  = NAN;
static bool     sensorOk   = false;

// --- Callbacks required by kfs_esp_common.h ---

void kfsReadSensor() {
    uint16_t co2 = 0;
    float temp = 0.0f;
    float hum = 0.0f;
    bool dataReady = false;

    uint16_t err = scd4x.getDataReadyStatus(dataReady);
    if (err || !dataReady) {
        // Not ready yet — skip this cycle, will retry next interval
        return;
    }

    err = scd4x.readMeasurement(co2, temp, hum);
    if (err || co2 == 0) {
        Serial.println("SCD41 read error");
        sensorOk = false;
        return;
    }

    sensorCo2  = co2;
    sensorTemp = temp;
    sensorHum  = hum;
    sensorOk   = true;

    Serial.printf("CO2: %u ppm  %.1f C  %.1f %%\n", co2, temp, hum);
}

String kfsDataJson() {
    if (!sensorOk) {
        return "\"co2\":null,\"temperature\":null,\"humidity\":null";
    }
    return "\"co2\":" + String(sensorCo2) +
           ",\"temperature\":" + String(sensorTemp, 1) +
           ",\"humidity\":" + String(sensorHum, 1);
}

String kfsDataHtml() {
    if (!sensorOk) {
        return "<div class='card'>"
               "<div class='label'>Sensor</div>"
               "<div class='value' style='color:#ef4444'>No data</div>"
               "</div>";
    }

    // CO2 color thresholds: green <800, yellow <1200, red >=1200
    String co2Color = "#22c55e";  // green
    if (sensorCo2 >= 1200) co2Color = "#ef4444";       // red
    else if (sensorCo2 >= 800) co2Color = "#eab308";   // yellow

    return "<div class='card'>"
           "<div class='label'>CO₂</div>"
           "<div class='value' style='color:" + co2Color + "'>" + String(sensorCo2) + " ppm</div>"
           "</div>"
           "<div class='card'>"
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
    Wire.begin(SCD41_SDA, SCD41_SCL);
    kfsSetup();
    delay(100);

    scd4x.begin(Wire, 0x62);

    // Stop any previous measurement (required before config changes)
    scd4x.stopPeriodicMeasurement();
    delay(500);

    // Optional: read serial number to verify communication
    uint64_t serialNumber = 0;
    uint16_t err = scd4x.getSerialNumber(serialNumber);
    if (err) {
        Serial.println("SCD41 not found! Check wiring.");
    } else {
        Serial.printf("SCD41 serial: %llu\n", serialNumber);
    }

    // Start periodic measurement (one reading every ~5s)
    err = scd4x.startPeriodicMeasurement();
    if (err) {
        Serial.println("SCD41 startPeriodicMeasurement failed!");
    } else {
        Serial.println("SCD41 periodic measurement started");
    }
}

void loop() {
    kfsLoop();
}
