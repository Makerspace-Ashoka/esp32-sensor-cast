#include "LittleFS.h"
#include "FS.h"
#include "DHT.h"

#define DHTTYPE DHT22

int dhtPin=5;

int wait=15000;

DHT dht(dhtPin, DHTTYPE);

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("- message appended");
    } else {
        Serial.println("- append failed");
    }
    file.close();
}

void setup(){
  Serial.begin(115200);

  if(!LittleFS.begin()){
    Serial.println("Error occured while mounting LittleFS");
    return;
  }
  File file = LittleFS.open("/test.txt", "r");
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }
  
  Serial.println("File Content:");
  while(file.available()){
    // appendFile(LittleFS, "/test.txt", "\n\nYo, here's how you add stuff here\n\nWell Done MF\r\n"); //Append some text to the previous file
    
    Serial.write(file.read());
  }
  file.close();

  Serial.println("DHT Readings:");
  dht.begin();
}
 
void loop() {

delay(wait);

  float h=dht.readHumidity();
float t=dht.readTemperature();

if(isnan(h)|isnan(t)){
  Serial.println("Failed to read the sensor! Check the circuit for possible causes.");
return;
}

Serial.print("Temperature: ");
Serial.println(t);

Serial.print("Humidity: ");

Serial.println(h);

}
