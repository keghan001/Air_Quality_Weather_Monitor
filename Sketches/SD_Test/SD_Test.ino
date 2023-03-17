//SD Card includes
#include "FS.h"
#include "SD.h"
#include "SPI.h"


#define valSize 5
float values[valSize] = {24.53, 64.11, 74.00, 56.42, 20.53};
String sensorVals;
const char* sensorV;


void setup(){
    Serial.begin(115200);

    sdInit();
}

void loop(){
  // const char * dataString = "Kwesi Manu Eghan\n";
  // appendFile(SD, "/datalogger.txt", dataString);

  for(int i=0; i<valSize; i++){
    values[i] += 1.5;
  }


  //Readings are stored in the sensorVals as a String
  //And converted to const char* in the sensorV variable for parsing
  sensorVals = valToString(values);
  // sensorV = valToString(values).c_str();
  sensorV = sensorVals.c_str();
  
  appendFile(SD, "/datalog.csv", sensorV);
  delay(500);

}



//Takes in an array of floats and converts to String 
String valToString(float vals[]){
  String values;
  
  for(int i=0; i<valSize; i++){
    values += (String)vals[i];
    
    if(i != valSize-1){
      values += ",";
    }
  }
  
  values += "\n";
  return values;
}





//Appending data to a file in the SD card
void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}



//Initializing the SD Card Shield in the setup function
void sdInit(){
  Serial.println("Initializing SD card...");
    if(!SD.begin()){
        Serial.println("Card Mount Failed");
        return;
    }
    Serial.println("card initialized.");
}
