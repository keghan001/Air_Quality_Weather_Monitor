//General declarations
#include <Wire.h>
//

//1. Temperature, Humidity Sensor declaration
#include "DHT.h"
#define Type DHT22
#define sensePin 25 // This pin is 02 on the UNO
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
#include <WiFiManager.h>
int status = WL_IDLE_STATUS;     // the Wifi radio's status


// 9. Declaration for the ATH20 sensor
// #include <AHT20.h>
// AHT20 aht20;

// 10. For the remote control
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#define kRecvPin 26
IRrecv irrecv(kRecvPin);
decode_results cmd;


//Resetting the device using software
void(* resetSystem) (void) = 0;  // declare reset fuction at address 0


//Adafruit IO for IoT hosting
#include "AdafruitIO_WiFi.h"
#define IO_USERNAME  "Keghan"
#define IO_KEY       "aio_LLOy92XJIqNScL3obmlu5wvTT6Ek"
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, "", "");
//Feeds for the IOT
AdafruitIO_Feed *temperatureFeed = io.feed("airq-sensors.temperature");
AdafruitIO_Feed *humidityFeed = io.feed("airq-sensors.humidity");
AdafruitIO_Feed *co2Feed = io.feed("airq-sensors.co2-conc");
AdafruitIO_Feed *coFeed = io.feed("airq-sensors.co-conc");
AdafruitIO_Feed *uvFeed = io.feed("airq-sensors.uvindex");
AdafruitIO_Feed *pm25Feed = io.feed("airq-sensors.pm2-5");
AdafruitIO_Feed *pm10Feed = io.feed("airq-sensors.pm10");
AdafruitIO_Feed *remoteFeed = io.feed("airq-sensors.remote");


//Metro Class for timing and synchronization
#include <Metro.h>
//Task timers for metro tasks
Metro taskLcdShow = Metro(100); // For the Lcd display of results
Metro taskWriteSD = Metro(15000); // For writing to SD card
Metro  taskRemoteRec = Metro(100); // For the remote control
Metro  taskAdaTransmit = Metro(2000); // For sending data to Adafruit IO


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
float pm1;
float pm25;
float pm10;


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
int alertPin = 12; // pin 8 on UNO

void beepAlert(){
  digitalWrite(alertPin, HIGH);
  delay(200);
  digitalWrite(alertPin, LOW);
}

void alert(int alertPin, int repsAlert=1, int waitAlert=1000) {
  int reps = repsAlert;
  int waits = waitAlert;

  for(int i=0; i<reps; i++){
    digitalWrite(alertPin, HIGH);
    lcd.noBacklight();
    delay(waits);
    lcd.backlight();
    digitalWrite(alertPin, LOW);
    delay(waits);
  }
  
  lcd.backlight();
}



void setup(){
    Serial.begin(115200);
    Wire.begin();
    lcd.begin(); // Lcd display initializer
    //introLcd(); // Lcd Intro messages

    sensorMsg("INITIALIZING SENSORS", "PLEASE WAIT...");
    delay(1500);
    irrecv.enableIRIn();  // Start the receiver
    initRtc(); //Initializing the rtc
    HT.begin(); //DHT initializer
    initSD(); //SD card initializer
    initBmp(); // Pressure sensor initializer
    // initAth20(); // ATH20 sensor initializer
    initGasSensor(); // Gas sensor initializer
    //delay(10000);
    initWireless(); //Initializes the Wifi
    pinMode(uvPin, INPUT); // Activating the uv pin mode
    pinMode(alertPin, OUTPUT); // Activating the alert pin
    
    sensorMsg("INITIALIZATION DONE!", "PROGRAM STARTING...");
    alert(alertPin, 5, 400);

    lcd.clear();
}

//Main Program
void loop(){

  //Starting iot
  io.run();

  //Checking for remote values using Metro tasks
  if(taskRemoteRec.check()){
    remoteCheck();
  }

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
  // appendFile(SD, "/datalog.csv", sensorCharV);

  //Using Metro to append file to SD card
  if(taskWriteSD.check()){
    appendFile(SD, "/datalog.csv", sensorCharV);
  }

  //Writing to lcd
  lcdShow(temp, humidity, pressure, uvIndex, CO, CO2);

  //Using metro to show lcd display
  // if(taskLcdShow.check()){
  //   lcdShow(temp, humidity, pressure, uvIndex, CO, CO2);
  // }

  if(taskAdaTransmit.check()){
    temperatureFeed->save(temp);
    humidityFeed->save(humidity);
    co2Feed->save(CO2);
    coFeed->save(CO);
    uvFeed->save((int)uvIndex);
  
  }


  delay(500); //This delay is now in the lcdShow function
}
// End of main program



//Initializing the wifi on the ESP-32
void initWireless(){
  lcd.clear();
  // attempt to connect to Wifi network:
  sensorMsg("CONNECTING TO WIFI", "PLEASE WAIT...");
  WiFiManager wm;
  wm.setClass("invert");          // enable "dark mode" for the config portal
  wm.setConfigPortalTimeout(120); // auto close configportal after n seconds
  wm.setAPClientCheck(true);      // avoid timeout if client connected to softap



  bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  res = wm.autoConnect("AirQ Monitor","kegman00"); // password protected ap

  if(!res) {
        Serial.println("Failed to connect");
        sensorMsg("WIFI CAN'T CONNECT!!", "TRY AGAIN LATER");
        // ESP.restart();
  } 
  else {
        //if you get here you have connected to the WiFi   
        String ssid = wm.getWiFiSSID(); 
        Serial.println("You're connected to the network " +  ssid );
        sensorMsg("WIFI CONNECTION MADE", "TO: " + ssid);
        delay(2000);

        sensorMsg("CONNECTING IOT HOST", "ADAFRUIT IO CLOUD");
        Serial.printf("Connecting to Adafruit IO with User: %s Key: %s.\n", IO_USERNAME, IO_KEY);
        io.connect();
        
        while ((io.status() < AIO_CONNECTED)){
          Serial.print(".");
          delay(500);
        }
        Serial.println("Connected to Adafruit IO.");

      }



  delay(1500);
  lcd.clear();
}



//Displays the active message
void activeMsg(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("WEATHER_AIR_QUALITY");
  lcd.setCursor(0,1);
  lcd.print("MONITORING SYSTEM");
  lcd.setCursor(0,2);
  lcd.print("DESIGNED BY KEGHAN");
  lcd.setCursor(0,3);
  lcd.print("--------------------");
}

//Sensors Calibrating messages
void sensorMsg(String act, String msg){
  //Lcd initialization message
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("--------------------");
  lcd.setCursor(0,1);
  lcd.print(act);
  lcd.setCursor(0,2);
  lcd.print(msg);
  lcd.setCursor(0,3);
  lcd.print("--------------------");
}


// Using the IR remote to receive various commands \786
void remoteCheck(){
  if (irrecv.decode(&cmd)) {
    if (cmd.value == 0xFFA25D){ // CH- on remote
      beepAlert();
      Serial.println("Device restarting\n");
      sensorMsg("DEVICE IS RESTARTING", "PLEASE WAIT...");
      delay(2000);
      resetSystem(); //call reset
    }
    else if (cmd.value == 0xFF629D){ //CH on remote
      beepAlert();
      Serial.println("Device restarting\n");
      sensorMsg("DEVICE IS RESTARTING", "PLEASE WAIT...");
      delay(2000);
      resetSystem(); //call reset
    }
    //For turning on and off the Lcd backlight
    else if (cmd.value == 0xFFA857){ //VOL+ on remote
      beepAlert();
      lcd.backlight(); // Turn On backlight
    }
    else if (cmd.value == 0xFFE01F){ //VOL- on remote
      beepAlert();
      lcd.noBacklight(); // Turn Off backlight
    }
    else if (cmd.value == 0xFF906F){ //EQ on remote
      beepAlert();
      lcd.noDisplay(); //turns display off
    }
    else if (cmd.value == 0xFFC23D){ //PLAY on remote
      beepAlert();
      lcd.display(); // turns display on
    }
    else if (cmd.value == 0xFF6897){ // 0 on the remote
      beepAlert();
      activeMsg(); // displays active message
      delay(3500);
      lcd.clear();
    }
    else if (cmd.value == 0xFF9867){ //100+ on remote
      beepAlert();
      delay(4000); // pauses program for 4s
    }
    else if (cmd.value == 0xFFB04F){ // 200+ on remote
      beepAlert();
      delay(10000); //pauses program for 10s
    }
     else if (cmd.value == 0xFF52AD){ // 9 on the remote  
      beepAlert();
      // Reinitializes sensors and other components
      HT.begin(); //DHT initializer
      initSD(); //SD card initializer
      initBmp(); // Pressure sensor initializer
      initGasSensor(); // Gas sensor initializer
      // initAth20(); // ATH20 sensor initializer
    }
    else if (cmd.value == 0xFF42BD){ // 7 on the remote 
      beepAlert();
      initWireless(); // Reinitializes the WiFi
    }

    
    irrecv.resume();  // Receive the next value
  }

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
    
  sensorMsg("INITIALIZATION", "SD CARD..."); //Lcd initialization message

  Serial.println("Initializing SD card...");
    if(!SD.begin()){
        Serial.println("Card Mount Failed");
        return;
    }
    Serial.println("card initialized.");

    delay(850);
    lcd.clear();
}

//2. Initializing the pressure sensor 
void initBmp(){
 
  sensorMsg("CALIBRATING", "PRESSURE SENSOR..."); //Lcd initialization message

  bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  Serial.println("BMP280 has been initialised succesfully!");
  delay(850);
  lcd.clear();
}


//3. Initializes the Gas sensor MQ-135 to work with various gases
void initGasSensor(){

  sensorMsg("INITIALIZATION", "GAS SENSOR...");  //Lcd initialization message


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

    delay(10000);
    lcd.clear();
  }


// 4. Initializes the Rtc on the Shield

void initRtc(){

  sensorMsg("INITIALIZATION", "REAL TIME CLOCK...");  //Lcd initialization message

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
  Serial.println("RTC initialized succesfully");
  delay(850);
  lcd.clear();
}


// 5. Initializing the ATH20 sensor
// void initAth20(){
//   //Check if the AHT20 will acknowledge

//   // sensorMsg("INITIALIZATION", "AHT 20 Sensor...");  //Lcd initialization message

//   if (aht20.begin() == false)
//   {
//     Serial.println("AHT20 not detected. Please check wiring. Freezing.");
//     while (1);
//   }
//   Serial.println("AHT20 acknowledged.");

//   delay(850)
//   lcd.clear();
// }


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
  lcd.print("AGYEI KWAKU DARKO");
  lcd.setCursor(0,3);
  delay(1000);
  lcd.print("EMMANUEL OPOKU");
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
  // float pm25, float pm10
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
            // lcd.print("PM2.5=" + String(pm25));
            // lcd.setCursor(14,1);
            // lcd.print("PM10=" + String(pm10));

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


            //delay(550);
            // lcd.clear();

}


//Reading and converting to UV index from the Guda UV sensor 
int readUV(int sensorPin){
  int index = 0;
  int sensorValue = 0;
  
  sensorValue = analogRead(sensorPin);  //connect UV sensor to Analog A1 on UNO  
  int voltage = (sensorValue/4095.0*3.3) *1000;  //Voltage in miliVolts
  
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


