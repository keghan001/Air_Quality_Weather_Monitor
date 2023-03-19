//Declaration for the ATH20 sensor
#include <AHT20.h>
AHT20 aht20;


void setup() {
  Serial.begin(115200);
  Serial.println("Humidity AHT20 examples");

  Wire.begin(); //Join I2C bus
  initAth20();

}

void loop() {
      //Get the new temperature and humidity value
    float temperature = aht20.getTemperature();
    float humidity = aht20.getHumidity();

    //Print the results
    Serial.print("Temperature: ");
    Serial.print(temperature, 2);
    Serial.print(" C\t");
    Serial.print("Humidity: ");
    Serial.print(humidity, 2);
    Serial.print("% RH");

    Serial.println();

}


void initAth20(){
    //Check if the AHT20 will acknowledge
  if (aht20.begin() == false)
  {
    Serial.println("AHT20 not detected. Please check wiring. Freezing.");
    while (1);
  }
  Serial.println("AHT20 acknowledged.");
}
