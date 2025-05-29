#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <ESPmDNS.h>


#define DHTPIN 19         // Pin where the DHT sensor is connected
#define DHTTYPE DHT22     // DHT 22 (AM2302)

const char* ssid = yourWifiSsid;
const char* password = yourWifiPassword;

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

void handleSensor() {
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temp) || isnan(humidity)) {
    server.send(500, "application/json", "{\"error\":\"Sensor read failed\"}");
    return;
  }

  String json = "{";
  json += "\"temperature\":" + String(temp, 2) + ",";
  json += "\"humidity\":" + String(humidity, 2);
  json += "}";

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  dht.begin();
  delay(1000);

  WiFi.begin(ssid, password);
  Serial.println("Connecting to Wi-Fi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  if (!MDNS.begin("esp1")) {
  Serial.println("Error starting mDNS");
  return;
}
Serial.println("mDNS responder started");


  Serial.println("\nConnected to Wi-Fi!");
  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/sensor", handleSensor);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
