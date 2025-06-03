#include "LittleFS.h"

void setup() {
  Serial.begin(115200);

  if (!LittleFS.begin()) {
    Serial.println("Failed to mount LittleFS");
    return;
  }

  Serial.println("Formatting LittleFS...");
  LittleFS.format();
  Serial.println("Filesystem formatted.");
}

void loop() {
  // Do nothing
}
