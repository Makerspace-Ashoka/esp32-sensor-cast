#include "LittleFS.h"
#include "FS.h"
#include "DHT.h"
#include <ArduinoJson.h>
#include "time.h"
#include <WiFi.h>

#define DHTTYPE DHT22
int dhtPin = 5;
int wait = 15000;

const char* ssid     = "RedBrick Waddles";
const char* password = "Daisy-Donaldo-Quacks?";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;  // IST = GMT+5:30 => 5*3600 + 30*60 = 19800
const int daylightOffset_sec = 0;

DHT dht(dhtPin, DHTTYPE);
const char* filename = "/log.json";

DynamicJsonDocument loadLog(fs::FS &fs, const char* path) {
    DynamicJsonDocument doc(2048);
    
    if (!fs.exists(path)) {
        Serial.println("Log file doesn't exist. Creating new log.");
        return doc;
    }

    File file = fs.open(path, "r");
    if (!file) {
        Serial.println("Failed to open log file for reading.");
        return doc;
    }

    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.c_str());
    }
    file.close();
    return doc;
}

void saveLog(fs::FS &fs, const char* path, DynamicJsonDocument& doc) {
    File file = fs.open(path, "w");
    if (!file) {
        Serial.println("Failed to open file for writing.");
        return;
    }
    if (serializeJsonPretty(doc, file)) {
        Serial.println("Log updated successfully.");
    } else {
        Serial.println("Failed to write to file.");
    }
    file.close();
}

void setup(){
    Serial.begin(115200);

    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected.");

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Serial.println("Waiting for NTP time sync...");
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nTime sync complete.");

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

    char timestamp[25];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);

    Serial.printf("Temp: %.2f°C, Humidity: %.2f%% at %s\n", t, h, timestamp);

    DynamicJsonDocument doc = loadLog(LittleFS, filename);

    int nextIndex = 0;
    for (JsonPair kv : doc.as<JsonObject>()) {
        int index = String(kv.key().c_str()).toInt();
        if (index >= nextIndex) nextIndex = index + 1;
    }

    JsonObject newEntry = doc.createNestedObject(String(nextIndex));
    newEntry["temperature"] = t;
    newEntry["humidity"] = h;
    newEntry["timestamp"] = timestamp;  // Add timestamp in IST

    saveLog(LittleFS, filename, doc);
}
