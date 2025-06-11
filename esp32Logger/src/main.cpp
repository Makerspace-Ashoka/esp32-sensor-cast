#include "LittleFS.h"
#include "FS.h"
#include "DHT.h"
#include <ArduinoJson.h>
#include "time.h"
#include <WiFi.h>
#include <HTTPClient.h>

#define DHTTYPE DHT22
int dhtPin =33;
int wait = 15000;

const char* ssid     = "RedBrick Waddles";
const char* password = "Daisy-Donaldo-Quacks?";
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800; // IST = UTC+5:30
const int daylightOffset_sec = 0;

DHT dht(dhtPin, DHTTYPE);
const char* filename = "/log.json";

DynamicJsonDocument loadLog(fs::FS &fs, const char* path) {
    DynamicJsonDocument doc(2048);
    if (!fs.exists(path)) return doc;

    File file = fs.open(path, "r");
    if (!file) return doc;

    DeserializationError error = deserializeJson(doc, file);
    file.close();
    return doc;
}

void saveLog(fs::FS &fs, const char* path, DynamicJsonDocument& doc) {
    File file = fs.open(path, "w");
    if (!file) return;
    serializeJsonPretty(doc, file);
    file.close();
}

void setup(){
    Serial.begin(115200);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected.");

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    if (!LittleFS.begin()) {
        Serial.println("LittleFS mount failed.");
        return;
    }

    dht.begin();
}

void loop() {
    delay(wait);

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
        Serial.println("Failed to read from DHT sensor.");
        return;
    }

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time.");
        return;
    }

    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);

    Serial.printf("Temp: %.2f°C, Humidity: %.2f%% @ %s\n", t, h, timestamp);

    // Load existing log
    DynamicJsonDocument doc = loadLog(LittleFS, filename);

    // Determine next index
    int nextIndex = 0;
    for (JsonPair kv : doc.as<JsonObject>()) {
        int index = String(kv.key().c_str()).toInt();
        if (index >= nextIndex) nextIndex = index + 1;
    }

    // Add new reading to local log
    JsonObject newEntry = doc.createNestedObject(String(nextIndex));
    newEntry["temperature"] = t;
    newEntry["humidity"] = h;
    newEntry["timestamp"] = timestamp;

    // Save to LittleFS
    saveLog(LittleFS, filename, doc);

    // Send to server
    StaticJsonDocument<256> payload;
    payload["temperature"] = t;
    payload["humidity"] = h;
    payload["timestamp"] = timestamp;

    String jsonPayload;
    serializeJson(payload, jsonPayload);

    HTTPClient http;
    http.begin("http://10.42.206.85:5000/upload");
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(jsonPayload);
    if (httpResponseCode > 0) {
        Serial.printf("Posted to server: %d\n", httpResponseCode);
    } else {
        Serial.printf("POST failed: %s\n", http.errorToString(httpResponseCode).c_str());
    }
    http.end();
}
