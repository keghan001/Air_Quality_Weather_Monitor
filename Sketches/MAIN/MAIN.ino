//General declarations
#include <Wire.h>

//1. Temperature, Humidity Sensor declaration
#include "DHT.h"
#define Type DHT22
#define sensePin 14
DHT HT(sensePin, Type);

//2. SD Card declarations
#include "FS.h"
#include "SD.h"
#include "SPI.h"

//3. BMP pressure sensor declarations 
#include <Adafruit_BMP280.h>
Adafruit_BMP280 bmp;

//4. LCD display declarations
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);


//5. MQ-135 Sensor declarations
#include <MQUnifiedsensor.h>
#define placa "ESP32"
#define Voltage_Resolution 5
#define pin 2 //Analog input 0 of your arduino
#define type "MQ-135" //MQ135
#define ADC_Bit_Resolution 10 // For arduino UNO/MEGA/NANO
#define RatioMQ135CleanAir 3.6//RS / R0 = 3.6 ppm  
//Declare Sensor
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);


//Handling the readings results
#define valSize 7
float valuesArray[valSize];
String sensorStrVals;
const char* sensorCharV;


//SENSOR VARIABLES
float humidity;
float temp;
float pressure;
float altitude;
float CO;
float CO2;


//Placing values into the array
void valueToArray(float arrayVals[valSize], float temp, float humidity, float pressure, 
                  float altitude, float CO, float CO2, float NH4) {
  arrayVals[0] = temp;
  arrayVals[1] = humidity;
  arrayVals[2] = pressure;
  arrayVals[3] = altitude;
  arrayVals[4] = CO;
  arrayVals[5] = CO2;
  arrayVals[6] = NH4;
}


void setup(){
    Serial.begin(115200);
    introLcd(); // Lcd Intro messages
    HT.begin(); //DHT initializer
    initSD(); //SD card initializer
    initBmp(); // Pressure sensor initializer
    initGasSensor(); // Gas sensor initializer
    lcd.begin(); // Lcd display initializer
    


}

//Main Program
void loop(){

  //Temperature and humidity readings
  humidity = HT.readHumidity(); // Readings are in percentage (%)
  temp = HT.readTemperature(); // Takes readings in Celcius (C)

  //Pressure and Altitude readings
  pressure = bmp.readPressure(); //Takes readings in Pascal (Pa)
  altitude = bmp.readAltitude(1013.25); // Takes readings in meters (m)

  //Gas sensor readings (CO, CO2, NH4)
  MQ135.update(); // Update data, the arduino will read the voltage from the analog pin
  MQ135.setA(605.18); MQ135.setB(-3.937); // Configure the equation to calculate CO concentration value
  CO = MQ135.readSensor();
  MQ135.setA(110.47); MQ135.setB(-2.862); // Configure for CO2
  CO2 = MQ135.readSensor() + 400;
  MQ135.setA(102.2 ); MQ135.setB(-2.473); // Configure for NH4
  float NH4 = MQ135.readSensor();



  // Placing values into the array
  valueToArray(valuesArray, temp, humidity, pressure, altitude, CO, CO2, NH4);


  //Converting sensor values from the array->String->Const char 
  sensorStrVals= valToString(valuesArray);
  sensorCharV = sensorStrVals.c_str();
  
  appendFile(SD, "/datalog.csv", sensorCharV);
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



//INITIALIZATIONS OF SENSORS AND COMPONENTS

//1. Initializing the SD Card Shield in the setup function
void initSD(){
  Serial.println("Initializing SD card...");
    if(!SD.begin()){
        Serial.println("Card Mount Failed");
        return;
    }
    Serial.println("card initialized.");
}

//2. Initializing the pressure sensor 
void initBmp(){
  bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}


//3. Initializes the Gas sensor MQ-135 to work with various gases
void initGasSensor(){
    MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
    
    MQ135.init(); 
    Serial.print("Calibrating please wait.");
    float calcR0 = 0;
    for(int i = 1; i<=10; i ++)
    {
      MQ135.update(); // Update data, the arduino will read the voltage from the analog pin
      calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
      Serial.print(".");
    }
    MQ135.setR0(calcR0/10);
    Serial.println("  done!.");

        /*
        Exponential regression:
      GAS      | a      | b
      CO       | 605.18 | -3.937  
      CO2      | 110.47 | -2.862
      NH4      | 102.2  | -2.473
      */
    
    if(isinf(calcR0)) {Serial.println("Warning: Conection issue, (Open circuit detected)"); while(1);}
    if(calcR0 == 0){Serial.println("Warning: Conection issue found, (Analog pin shorts to ground)"); while(1);}
  }






//LCD welcome intro messages 
void introLcd(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Air Quality Monitor");
  lcd.setCursor(0,1);
  lcd.print("Physics Department");
  lcd.setCursor(0,2);
  lcd.print("KNUST, KUMASI CAMPUS");
  lcd.setCursor(0,3);
  lcd.print("Prototype: Mac 1");

  delay(5000);
  lcd.clear();


  lcd.setCursor(0,0);
  lcd.print("----- STUDENTS -----");
  delay(850);
  lcd.setCursor(0,1);
  lcd.print("Kwesi Manu Eghan");
  lcd.setCursor(0,2);
  delay(1000);
  lcd.print("Emmanuel Opoku");
  lcd.setCursor(0,3);
  delay(1000);
  lcd.print("Agyei Kwaku Darko");
  delay(3500);
  lcd.clear();


  lcd.setCursor(0,0);
  lcd.print("----SUPERVISORS-----");
  delay(850);
  lcd.setCursor(0,1);
  lcd.print("Prof. F. K Ampong");
  lcd.setCursor(0,2);
  delay(1000);
  lcd.print("Dr. J.N.A Aryee");
  delay(3500);
  lcd.clear();
}
