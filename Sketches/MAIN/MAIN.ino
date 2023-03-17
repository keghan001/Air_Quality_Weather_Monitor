//General declarations
#include <Wire.h>
//

//1. Temperature, Humidity Sensor declaration
#include "DHT.h"
#define Type DHT22
#define sensePin 26 // This pin is 02 on the UNO
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
#define placa "ESP-32"
#define Voltage_Resolution 3.3 // 3V3 <- IMPORTANT
#define gasPin 35 //Analog input A2 of your arduino
#define gasType "MQ-135" //MQ135
#define ADC_Bit_Resolution 12 // ESP-32 bit resolution
#define RatioMQ135CleanAir 3.6//RS / R0 = 3.6 ppm  
//Declare Sensor
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, gasPin, gasType);

// 6. UV Sensor declarations using an analog pin
int uvPin = 34;   // select the input for the UV sensor pin A3 on UNO


// 7. RTC declarations
#include "RTClib.h"
RTC_DS1307 rtc;
String timeStamp;


//8. WiFi declarations
#include "WiFi.h"
#include "secrets.h"
const char* ssid = SECRET_SSID;     //  your network SSID (name)
const char* pass = SECRET_PASS;  // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status


//Handling the readings results
#define valSize 8
float valuesArray[valSize];
String sensorStrVals;
const char* sensorCharV;


//SENSOR VARIABLES
float temp;
float humidity;
float pressure;
float altitude;
float CO;
float CO2;
float NH4;
float uvIndex;


//Placing values into the array
void valueToArray(float arrayVals[valSize], float temp, float humidity, float pressure, 
                  float altitude, float CO, float CO2, float NH4, float uvIndex) {
  arrayVals[0] = temp;
  arrayVals[1] = humidity;
  arrayVals[2] = pressure;
  arrayVals[3] = altitude;
  arrayVals[4] = CO;
  arrayVals[5] = CO2;
  arrayVals[6] = NH4;
  arrayVals[7] = uvIndex;
}

//For alerting using both an LED and an active buzzer
int alertPin = 17; // pin 4 on UNO
void alert(int alertPin, int repsAlert=1, int waitAlert=1000) {
  int reps = repsAlert;
  int waits = waitAlert;
  for(int i=0; i<reps; i++){
    digitalWrite(alertPin, HIGH);
    delay(waits);
    digitalWrite(alertPin, LOW);
    delay(waits);
  }
}



void setup(){
    Serial.begin(115200);
    Wire.begin();
    lcd.begin(); // Lcd display initializer
    //introLcd(); // Lcd Intro messages

    initMsg();
    delay(1500);

    initRtc(); //Initializing the rtc
    HT.begin(); //DHT initializer
    initSD(); //SD card initializer
    initBmp(); // Pressure sensor initializer
    initGasSensor(); // Gas sensor initializer
    pinMode(uvPin, INPUT); // Activating the uv pin mode
    pinMode(alertPin, OUTPUT); // Activating the alert pin
    initWireless(); //Initializes the Wifi
    endMsg();
    alert(alertPin, 5, 400);

    lcd.clear();
}

//Main Program
void loop(){

  //Reads the timestamp from the function and converts to const char
  timeStamp = dataTime();

  //Temperature and humidity readings
  temp = bmp.readTemperature(); // Takes readings in Celcius (C)
  humidity = HT.readHumidity(); // Readings are in percentage (%)
  
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
  NH4 = MQ135.readSensor();

  //Reading the uv index
  uvIndex = readUV(uvPin);

   


  // Placing values into the array
  valueToArray(valuesArray, temp, humidity, pressure, altitude, CO, CO2, NH4, uvIndex);


  //Converting sensor values from the array->String->Const char 
  sensorStrVals= timeStamp + valToString(valuesArray);
  sensorCharV = sensorStrVals.c_str();
  
  Serial.println(sensorStrVals);

  //Appending cont char* readings values to the file in SD
  appendFile(SD, "/datalog.csv", sensorCharV);

  //Writing to lcd
  lcdShow(temp, humidity, pressure, uvIndex, CO, CO2);

  //delay(2000);
}
// End of main program



//Initializing the wifi on the ESP-32
void initWireless(){
  lcd.clear();
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
  Serial.print("Attempting to connect to WPA SSID: ");
  lcd.setCursor(0, 0);
  lcd.print("CONNECTING TO WIFI..");
  lcd.setCursor(0, 1);
  lcd.print("NAME: " + String(ssid));
  lcd.setCursor(0, 2);
  lcd.print("PLEASE WAIT ...");
  Serial.println(ssid);

  // Connect to WPA/WPA2 network:
  status = WiFi.begin(ssid, pass);
      // wait 10 seconds for connection:
    delay(5000);
  }

  // you're connected now, so print out the data:
  Serial.print("You're connected to the network " +  String(ssid) + "\n");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("--------------------");
  lcd.setCursor(0,1);
  lcd.print("WIFI CONNECTION MADE");
  lcd.setCursor(0,2);
  lcd.print("TO " + String(ssid) + "!");
  lcd.setCursor(0,3);
  lcd.print("--------------------");

  delay(3000);
  lcd.clear();
}


//Message when sensors and other components are initializing
void initMsg(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("--------------------");
  lcd.setCursor(0,1);
  lcd.print("INITIALIZING SENSORS");
  lcd.setCursor(0,2);
  lcd.print("PLEASE WAIT...");
  lcd.setCursor(0,3);
  lcd.print("--------------------");
}

//Message when sensors and other components have been initialized
void endMsg(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("--------------------");
  lcd.setCursor(0,1);
  lcd.print("INITIALIZATION DONE!");
  lcd.setCursor(0,2);
  lcd.print("PROGRAM STARTING...");
  lcd.setCursor(0,3);
  lcd.print("--------------------");
}

//Sensors Calibrating messages
void sensorMsg(String msg){
  //Lcd initialization message
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("--------------------");
  lcd.setCursor(0,1);
  lcd.print("INITIALIZING");
  lcd.setCursor(0,2);
  lcd.print(msg);
  lcd.setCursor(0,3);
  lcd.print("--------------------");
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


// This gets the timestamp and returns it as a String
String dataTime() {
  DateTime now = rtc.now();
  String dattime = String(int(now.year())) +"-"+ String(int(now.month())) +"-"+ String(int(now.day()));
  dattime += " "+ String(int(now.hour())) +":"+ String(int(now.minute())) +":"+ String(int(now.second())) +",";

  return dattime;
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
    
  sensorMsg("SD CARD..."); //Lcd initialization message

  Serial.println("Initializing SD card...");
    if(!SD.begin()){
        Serial.println("Card Mount Failed");
        return;
    }
    Serial.println("card initialized.");

    delay(700);
    lcd.clear();
}

//2. Initializing the pressure sensor 
void initBmp(){
 
  sensorMsg("PRESSURE SENSOR..."); //Lcd initialization message

  bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */


  delay(700);
  lcd.clear();
}


//3. Initializes the Gas sensor MQ-135 to work with various gases
void initGasSensor(){

  sensorMsg("GAS SENSOR...");  //Lcd initialization message


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

    delay(700);
    lcd.clear();
  }


// 4. Initializes the Rtc on the Shield

void initRtc(){

  sensorMsg("REAL TIME CLOCK...");  //Lcd initialization message

    if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));

  delay(700);
  lcd.clear();
}




//LCD welcome intro messages 
void introLcd(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("AIR QUALITY MONITOR");
  lcd.setCursor(0,1);
  lcd.print("PHYSICS DEPARTMENT");
  lcd.setCursor(0,2);
  lcd.print("KNUST, KUMASI CAMPUS");
  lcd.setCursor(0,3);
  lcd.print("Prototype: Mac 1");

  delay(6000);
  lcd.clear();


  lcd.setCursor(0,0);
  lcd.print("----- STUDENTS -----");
  delay(850);
  lcd.setCursor(0,1);
  lcd.print("KWESI MANU EGHAN");
  lcd.setCursor(0,2);
  delay(1000);
  lcd.print("EMMANUEL OPOKU");
  lcd.setCursor(0,3);
  delay(1000);
  lcd.print("AGYEI KWAKU DARKO");
  delay(3000);
  lcd.clear();


  lcd.setCursor(0,0);
  lcd.print("--- SUPERVISORS ----");
  delay(850);
  lcd.setCursor(0,1);
  lcd.print("PROF. F. K AMPONG");
  lcd.setCursor(0,2);
  delay(1000);
  lcd.print("DR. J.N.A ARYEE");
  delay(6500);
  lcd.clear();
}


//Displaying sensor values on the 20x40 lcd display
void lcdShow(float temp, float humidity, float pressure, 
              float uvIndex, float CO, float CO2) {
  //
            //Temperature and Humidity ROW 1
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("TMP=" + String(temp) +"C");
            lcd.setCursor(11,0);
            lcd.print("HMD=" + String(humidity));

            //Pressure ROW 2
            lcd.setCursor(0,1);
            lcd.print("PRESS=" + String(pressure) + "Pa");
            // lcd.setCursor(0,1); // We'll choode particulate matter over pressure in final design
            // lcd.print("PM2.5=" + String(pmConc));

            //UV INDEX and CO ROW 3
            lcd.setCursor(0,2);
            lcd.print("UVI=" + String((int)(uvIndex)));
            lcd.setCursor(8,2);
            lcd.print("CO=" + String(CO) + "ppm");

            //CO2 and maybe NH4 ROW 4
            lcd.setCursor(0,3);
            lcd.print("CO2=" + String(CO2) + "ppm");
            // lcd.setCursor(13,3);
            // lcd.print("NH4=" + String(NH4));


            delay(1000);
            lcd.clear();

}


//Reading and converting to UV index from the Guda UV sensor 
int readUV(int sensorPin){
  int index = 0;
  int sensorValue = 0;
  
  sensorValue = analogRead(sensorPin);  //connect UV sensor to Analog A1 on UNO  
  int voltage = (sensorValue * (5.0 / 1023.0))*1000;  //Voltage in miliVolts
  
  if(voltage<50)
  {
    index = 0;
  }else if (voltage>50 && voltage<=227)
  {
    index = 0;
  }else if (voltage>227 && voltage<=318)
  {
    index = 1;
  }
  else if (voltage>318 && voltage<=408)
  {
    index = 2;
  }else if (voltage>408 && voltage<=503)
  {
    index = 3;
  }
  else if (voltage>503 && voltage<=606)
  {
    index = 4;
  }else if (voltage>606 && voltage<=696)
  {
    index = 5;
  }else if (voltage>696 && voltage<=795)
  {
    index = 6;
  }else if (voltage>795 && voltage<=881)
  {
    index = 7;
  }
  else if (voltage>881 && voltage<=976)
  {
    index = 8;
  }
  else if (voltage>976 && voltage<=1079)
  {
    index = 9;
  }
  else if (voltage>1079 && voltage<=1170)
  {
    index = 10;
  }else if (voltage>1170)
  {
    index = 11;
  }

  //Returning the uv index value
  return index;
}


