//Temperature, Humidity Sensor declaration
#include "DHT.h"
#define Type DHT22
#define sensePin 14
DHT HT(sensePin, Type);


float humidity;
float tempC;

void setup() {
  Serial.begin(115200);
  HT.begin();
  Serial.println(HT.readHumidity());
}

void loop() {
  humidity = HT.readHumidity();
  tempC = HT.readTemperature();

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("\tTemperature: ");
  Serial.println(tempC);


  delay(500);
 
}
