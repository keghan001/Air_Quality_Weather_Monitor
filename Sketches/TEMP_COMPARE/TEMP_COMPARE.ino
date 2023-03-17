//BMP pressure sensor declarations 
#include <Wire.h>
#include <Adafruit_BMP280.h>
Adafruit_BMP280 bmp;

//Temperature, Humidity Sensor declaration
#include "DHT.h"
#define Type DHT22
#define sensePin 14
DHT HT(sensePin, Type);


void setup() {
  Serial.begin(115200);
  initBmp();
  HT.begin();
}

void loop() {
    //Serial.print(F("Temp = "));
    Serial.print(bmp.readTemperature());
    Serial.print("\t | " );
    Serial.println(HT.readTemperature());



    Serial.println();
    delay(500);
}


//Initializing the pressure sensor 
void initBmp(){
  bmp.begin();

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}
