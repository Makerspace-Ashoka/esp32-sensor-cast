#include "LittleFS.h"
#include "FS.h"

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
}
 
void loop() {

}
