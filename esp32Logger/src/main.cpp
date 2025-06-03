#include "LittleFS.h"
#include "FS.h"
#include "DHT.h"
#include <ArduinoJson.h>  // Install via Library Manager

#define DHTTYPE DHT22
int dhtPin = 5;
int wait = 15000;

DHT dht(dhtPin, DHTTYPE);

const char* filename = "/log.json";

// Function to load existing JSON data
DynamicJsonDocument loadLog(fs::FS &fs, const char* path) {
    DynamicJsonDocument doc(2048);
    
    if (!fs.exists(path)) {
        Serial.println("Log file doesn't exist. Creating new log.");
        return doc;  // Return empty doc
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

// Function to save JSON data
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

    Serial.printf("Temp: %.2f°C, Humidity: %.2f%%\n", t, h);

    // Load existing log
    DynamicJsonDocument doc = loadLog(LittleFS, filename);

    // Determine next index
    int nextIndex = 0;
    for (JsonPair kv : doc.as<JsonObject>()) {
        int index = String(kv.key().c_str()).toInt();
        if (index >= nextIndex) nextIndex = index + 1;
    }

    // Add new reading
    JsonObject newEntry = doc.createNestedObject(String(nextIndex));
    newEntry["temperature"] = t;
    newEntry["humidity"] = h;

    // Save updated log
    saveLog(LittleFS, filename, doc);
}
