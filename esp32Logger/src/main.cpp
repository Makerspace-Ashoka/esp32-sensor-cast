#include "LittleFS.h"
#include "FS.h"
#include "DHT.h"
#include <ArduinoJson.h>
#include "time.h"
#include <WiFi.h>
#include <HTTPClient.h>

#define FORMAT_LITTLEFS_IF_FAILED true
#define DHTTYPE DHT22
int dhtPin = 33;
int wait = 60000;

#define NODE_NAME "esp32-node-1"

const char* ssid     = "RedBrick Waddles";
const char* password = "Daisy-Donaldo-Quacks?";
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800; // IST = UTC+5:30
const int daylightOffset_sec = 0;

const char* filename = "/log.json";
const char* serverURL = "http://10.42.207.234:5000/upload"; // Change if needed

DHT dht(dhtPin, DHTTYPE);

void sendReadingToServer(float temperature, float humidity, const char* timestamp) {
    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    JsonDocument doc;
    JsonObject obj = doc.to<JsonObject>();
    obj["node_name"] = NODE_NAME;
    obj["timestamp"] = timestamp;
    obj["temperature"] = temperature;
    obj["humidity"] = humidity;

    String requestBody;
    serializeJson(obj, requestBody);

    int httpResponseCode = http.POST(requestBody);
    if (httpResponseCode > 0) {
        Serial.printf("POST sent. Response code: %d\n", httpResponseCode);
        String response = http.getString();
        Serial.println("Server response:");
        Serial.println(response);
    } else {
        Serial.printf("POST failed. Error: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    http.end();
}

void appendReadingToFile(float temperature, float humidity, const char* timestamp) {
    JsonDocument doc;
    doc.to<JsonObject>();

    if (LittleFS.exists(filename)) {
        File file = LittleFS.open(filename, "r");
        if (file) {
            DeserializationError error = deserializeJson(doc, file);
            if (error) {
                Serial.println("Failed to parse existing log.json. Starting fresh.");
                doc.clear();
                doc.to<JsonObject>();
            }
            file.close();
        }
    }

    int nextIndex = 0;
    for (JsonPair kv : doc.as<JsonObject>()) {
        int index = String(kv.key().c_str()).toInt();
        if (index >= nextIndex) nextIndex = index + 1;
    }

    JsonObject entry = doc[String(nextIndex)].to<JsonObject>();
    entry["node_name"] = NODE_NAME;
    entry["timestamp"] = timestamp;
    entry["temperature"] = temperature;
    entry["humidity"] = humidity;

    File file = LittleFS.open(filename, "w");
    if (!file) {
        Serial.println("Failed to open log.json for writing.");
        return;
    }

    serializeJsonPretty(doc, file);
    file.close();

    String jsonOut;
    serializeJson(entry, jsonOut);
    Serial.print("Saved reading: ");
    Serial.println(jsonOut);

    // Post to server
    sendReadingToServer(temperature, humidity, timestamp);
}

void setup() {
    Serial.begin(115200);

    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
        Serial.println("LittleFS mount failed");
        return;
    }

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected.");

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    dht.begin();
    delay(wait);
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

    appendReadingToFile(t, h, timestamp);
}
