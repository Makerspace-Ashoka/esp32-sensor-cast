#include "LittleFS.h"
#include "FS.h"

void setup() {
  Serial.begin(115200);

  // Mount LittleFS
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount LittleFS");
    return;
  }

  // Open log.json
  File file = LittleFS.open("/log.json", "r");
  if (!file) {
    Serial.println("Failed to open log.json");
    return;
  }

  Serial.println("Contents of log.json:");

  // Print file contents line by line
  while (file.available()) {
    Serial.write(file.read());
  }

  file.close();
}

void loop() {
  // Do nothing
}
