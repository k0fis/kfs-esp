/*
 * kfs_esp_common.h — shared infrastructure for KFS ESP sensors
 *
 * WiFiManager captive portal, LittleFS config, HTTP server with
 * human-readable status page (/) and JSON API (/kfsEsp).
 *
 * Works on ESP8266 and ESP32.
 *
 * Usage in sensor sketch:
 *   #include "../shared/kfs_esp_common.h"
 *   #define SENSOR_TYPE "dht22"
 *   void kfsReadSensor();             // implement: update kfsData
 *   String kfsDataJson();             // implement: return JSON object body
 *   String kfsDataHtml();             // implement: return HTML snippet
 *   void setup() { kfsSetup(); }
 *   void loop()  { kfsLoop();  }
 */

#ifndef KFS_ESP_COMMON_H
#define KFS_ESP_COMMON_H

// --- Platform abstraction ---
#if defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  #define KFS_CHIP_ID  ((uint32_t)(ESP.getEfuseMac() & 0xFFFFFF))
  WebServer kfsServer(80);
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #define KFS_CHIP_ID  ESP.getChipId()
  ESP8266WebServer kfsServer(80);
#else
  #error "Unsupported platform — use ESP8266 or ESP32"
#endif

#include <WiFiManager.h>
#include <LittleFS.h>

// --- Configuration ---
#define KFS_AP_NAME       "KFS-ESP-Setup"
#define KFS_CONFIG_FILE   "/config.json"
#define KFS_NAME_MAX      64
#define KFS_SENSOR_INTERVAL 10000  // ms between sensor reads

// --- Globals ---
static char kfsDeviceName[KFS_NAME_MAX] = "";
static bool kfsShouldSaveConfig = false;
static unsigned long kfsLastSensorRead = 0;

// --- Forward declarations (implement in sensor sketch) ---
extern void kfsReadSensor();
extern String kfsDataJson();   // e.g. "\"temperature\":23.5,\"humidity\":48.2"
extern String kfsDataHtml();   // e.g. "<p>Temperature: 23.5 C</p>"

#ifndef SENSOR_TYPE
  #error "Define SENSOR_TYPE before including kfs_esp_common.h"
#endif

// --- Config persistence (LittleFS) ---

static void kfsLoadConfig() {
    if (!LittleFS.exists(KFS_CONFIG_FILE)) return;
    File f = LittleFS.open(KFS_CONFIG_FILE, "r");
    if (!f) return;

    String content = f.readString();
    f.close();

    // Minimal JSON parse — find "name":"value"
    int idx = content.indexOf("\"name\"");
    if (idx < 0) return;
    int q1 = content.indexOf('"', idx + 6);
    if (q1 < 0) return;
    int q2 = content.indexOf('"', q1 + 1);
    if (q2 < 0) return;

    String name = content.substring(q1 + 1, q2);
    name.toCharArray(kfsDeviceName, KFS_NAME_MAX);
}

static void kfsSaveConfig() {
    File f = LittleFS.open(KFS_CONFIG_FILE, "w");
    if (!f) {
        Serial.println("Failed to save config");
        return;
    }
    f.printf("{\"name\":\"%s\"}", kfsDeviceName);
    f.close();
    Serial.printf("Config saved: name=%s\n", kfsDeviceName);
}

static void kfsSaveConfigCallback() {
    kfsShouldSaveConfig = true;
}

// --- HTTP handlers ---

static void kfsHandleRoot() {
    unsigned long uptime = millis() / 1000;
    unsigned long days = uptime / 86400;
    unsigned long hours = (uptime % 86400) / 3600;
    unsigned long mins = (uptime % 3600) / 60;

    String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>"
                  "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                  "<title>" + String(kfsDeviceName) + "</title>"
                  "<style>"
                  "body{font-family:system-ui,sans-serif;max-width:480px;margin:40px auto;padding:0 20px;background:#1a1a2e;color:#e0e0e0}"
                  "h1{color:#7c3aed;margin-bottom:4px}"
                  ".type{color:#888;font-size:0.9em;margin-bottom:24px}"
                  ".card{background:#16213e;border-radius:8px;padding:16px;margin:12px 0}"
                  ".label{color:#888;font-size:0.85em}"
                  ".value{font-size:1.4em;font-weight:600}"
                  ".info{color:#666;font-size:0.8em;margin-top:20px}"
                  "</style></head><body>"
                  "<h1>" + String(kfsDeviceName) + "</h1>"
                  "<div class='type'>" SENSOR_TYPE "</div>"
                  + kfsDataHtml() +
                  "<div class='card'>"
                  "<div class='label'>Uptime</div>"
                  "<div class='value'>" + String(days) + "d " + String(hours) + "h " + String(mins) + "m</div>"
                  "</div>"
                  "<div class='info'>"
                  "IP: " + WiFi.localIP().toString() + "<br>"
                  "Free heap: " + String(ESP.getFreeHeap()) + " B<br>"
                  "RSSI: " + String(WiFi.RSSI()) + " dBm"
                  "</div>"
                  "</body></html>";

    kfsServer.send(200, "text/html", html);
}

static void kfsHandleApi() {
    unsigned long uptime = millis() / 1000;

    String json = "{"
                  "\"name\":\"" + String(kfsDeviceName) + "\","
                  "\"type\":\"" SENSOR_TYPE "\","
                  "\"data\":{" + kfsDataJson() + "},"
                  "\"uptime\":" + String(uptime) +
                  "}";

    kfsServer.send(200, "application/json", json);
}

static void kfsHandleNotFound() {
    kfsServer.send(404, "text/plain", "Not Found");
}

// --- Main setup/loop ---

static void kfsSetup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("KFS ESP " SENSOR_TYPE " starting...");

    // LittleFS
    if (!LittleFS.begin()) {
        Serial.println("LittleFS mount failed, formatting...");
        LittleFS.format();
        LittleFS.begin();
    }
    kfsLoadConfig();

    // Default name from chip ID
    if (strlen(kfsDeviceName) == 0) {
        snprintf(kfsDeviceName, KFS_NAME_MAX, "kfs-esp-%06x", KFS_CHIP_ID);
        Serial.printf("No saved name, using default: %s\n", kfsDeviceName);
    } else {
        Serial.printf("Loaded name: %s\n", kfsDeviceName);
    }

    // WiFiManager
    WiFiManager wm;
    wm.setConfigPortalTimeout(300);  // 5 min portal timeout, then retry
    wm.setSaveConfigCallback(kfsSaveConfigCallback);

    WiFiManagerParameter nameParam("name", "Device name", kfsDeviceName, KFS_NAME_MAX);
    wm.addParameter(&nameParam);

    if (!wm.autoConnect(KFS_AP_NAME)) {
        Serial.println("WiFi connect failed, restarting...");
        delay(3000);
        ESP.restart();
    }

    // Save config if changed via portal
    if (kfsShouldSaveConfig) {
        strncpy(kfsDeviceName, nameParam.getValue(), KFS_NAME_MAX - 1);
        kfsDeviceName[KFS_NAME_MAX - 1] = '\0';
        kfsSaveConfig();
    }

    Serial.printf("WiFi OK: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Device: %s (%s)\n", kfsDeviceName, SENSOR_TYPE);

    // Set hostname
    WiFi.hostname(kfsDeviceName);
#if defined(ESP32)
    WiFi.setHostname(kfsDeviceName);
#endif

    // Web server
    kfsServer.on("/", kfsHandleRoot);
    kfsServer.on("/kfsEsp", kfsHandleApi);
    kfsServer.onNotFound(kfsHandleNotFound);
    kfsServer.begin();
    Serial.println("HTTP server started on port 80");
}

static void kfsLoop() {
    kfsServer.handleClient();

    unsigned long now = millis();
    if (now - kfsLastSensorRead >= KFS_SENSOR_INTERVAL) {
        kfsLastSensorRead = now;
        kfsReadSensor();
    }
}

#endif // KFS_ESP_COMMON_H
