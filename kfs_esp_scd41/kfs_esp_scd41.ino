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
#define I2C_TIMEOUT_MS    100   // max time for a single I2C transaction
#define I2C_ERROR_LIMIT   30    // consecutive errors before I2C bus reset (~5 min at 10s interval)

SensirionI2cScd4x scd4x;

// --- Sensor data ---
static uint16_t sensorCo2  = 0;
static float    sensorTemp = NAN;
static float    sensorHum  = NAN;
static bool     sensorOk   = false;
static uint16_t i2cErrorCount = 0;

// --- I2C bus recovery ---

static void i2cReset() {
    Serial.println("I2C bus reset — toggling SCL");
    Wire.end();
    // Toggle SCL to release stuck SDA
    pinMode(SCD41_SCL, OUTPUT);
    for (int i = 0; i < 16; i++) {
        digitalWrite(SCD41_SCL, HIGH);
        delayMicroseconds(5);
        digitalWrite(SCD41_SCL, LOW);
        delayMicroseconds(5);
    }
    digitalWrite(SCD41_SCL, HIGH);
    pinMode(SCD41_SDA, INPUT_PULLUP);
    delay(50);
    Wire.begin(SCD41_SDA, SCD41_SCL);
    Wire.setTimeOut(I2C_TIMEOUT_MS);
    // Restart SCD41 measurement
    scd4x.stopPeriodicMeasurement();
    delay(500);
    scd4x.startPeriodicMeasurement();
    Serial.println("I2C bus reset complete, measurement restarted");
}

// --- Callbacks required by kfs_esp_common.h ---

void kfsReadSensor() {
    uint16_t co2 = 0;
    float temp = 0.0f;
    float hum = 0.0f;
    bool dataReady = false;

    uint16_t err = scd4x.getDataReadyStatus(dataReady);
    if (err || !dataReady) {
        if (err) {
            i2cErrorCount++;
            Serial.printf("SCD41 dataReady error (count: %u)\n", i2cErrorCount);
            if (i2cErrorCount >= I2C_ERROR_LIMIT) {
                i2cReset();
                i2cErrorCount = 0;
            }
        }
        return;
    }

    err = scd4x.readMeasurement(co2, temp, hum);
    if (err || co2 == 0) {
        Serial.println("SCD41 read error");
        i2cErrorCount++;
        sensorOk = false;
        if (i2cErrorCount >= I2C_ERROR_LIMIT) {
            i2cReset();
            i2cErrorCount = 0;
        }
        return;
    }

    i2cErrorCount = 0;  // success — reset counter
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
    Wire.setTimeOut(I2C_TIMEOUT_MS);
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
