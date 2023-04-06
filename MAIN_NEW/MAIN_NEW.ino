//General declarations
#include <Wire.h>
//

// 1. Declaration for the ATH20 sensor
#include <AHT20.h>
AHT20 aht20;

//2. SD Card declarations
#include "FS.h"
#include "SD.h"
#include "SPI.h"

bool fileSuccess;

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


//6. Serial Port connections for PM2.5 Sensor
#define RXD2 16 // To sensor TXD
#define TXD2 17 // To sensor RXD

struct pms5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};


struct pms5003data data;

// 7. RTC declarations
#include "RTClib.h"
RTC_DS1307 rtc;
String timeStamp;


// 8. For the remote control
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#define kRecvPin 26
IRrecv irrecv(kRecvPin);
decode_results cmd;


//Resetting the device using software
void(* resetSystem) (void) = 0;  // declare reset fuction at address 0



//Metro Class for timing and synchronization
#include <Metro.h>
//Task timers for metro tasks
Metro taskLcdShow = Metro(100); // For the Lcd display of results
Metro taskWriteSD = Metro(15000); // For writing to SD card
Metro  taskRemoteRec = Metro(100); // For the remote control


//Handling the readings results
#define valSize 10
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
float pm1;
float pm25;
float pm10;


//Placing values into the array
void valueToArray(float arrayVals[valSize], float temp, float humidity, float pressure, 
                  float altitude, float CO, float CO2, float NH4, float pm1, float pm25, float pm10) {
  arrayVals[0] = temp;
  arrayVals[1] = humidity;
  arrayVals[2] = pressure;
  arrayVals[3] = altitude;
  arrayVals[4] = CO;
  arrayVals[5] = CO2;
  arrayVals[6] = NH4;
  arrayVals[7] = pm1;
  arrayVals[8] = pm25;
  arrayVals[9] = pm10;
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
    initSD(); //SD card initializer
    initBmp(); // Pressure sensor initializer
    initAth20(); // ATH20 sensor initializer
    pinMode(alertPin, OUTPUT); // Activating the alert pin
    Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2); // Set up UART connection
    initGasSensor(); // Gas sensor initializer
    //delay(10000);
    
    
    sensorMsg("INITIALIZATION DONE!", "PROGRAM STARTING...");
    alert(alertPin, 5, 400);

    lcd.clear();
}

//Main Program
void loop(){

  //Checking for remote values using Metro tasks
  if(taskRemoteRec.check()){
    remoteCheck();
  }

  //Reads the timestamp from the function and converts to const char
  timeStamp = dataTime();

  //Temperature and humidity readings
  temp = aht20.getTemperature(); // Takes readings in Celcius (C)
  humidity = aht20.getHumidity(); // Readings are in percentage (%)
  
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

  // Taking particulate matter readings
  if (readPMSdata(&Serial1)) {
    pm1 = data.pm10_env;
    pm25 = data.pm25_env;
    pm10 = data.pm100_env;
  
  }
  

  // Placing values into the array
  valueToArray(valuesArray, temp, humidity, pressure, altitude, CO, CO2, NH4, pm1, pm25, pm10);


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
  lcdShow(temp, humidity, pressure, CO, CO2, pm1, pm25, pm10, altitude);


  delay(500); //This delay is now in the lcdShow function
}
// End of main program




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
      initSD(); //SD card initializer
      initBmp(); // Pressure sensor initializer
      initAth20(); // ATH20 sensor initializer
    }
    else if (cmd.value == 0xFF4AB5){ // 7 on the remote 
      beepAlert();
      initGasSensor(); // Gas sensor initializer
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
        fileSuccess = 0;
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
        fileSuccess = 1;
    } else {
        Serial.println("Append failed");
        fileSuccess = 0;
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

  bmp.begin(BMP280_ADDRESS, BMP280_CHIPID);

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

    //delay(10000);
    for(int i=20; i>0; i--){
      sensorMsg("INITIALIZATION", "GAS SENSOR... " + String(i) + "s");
      delay(1000);
    }
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
void initAth20(){
  //Check if the AHT20 will acknowledge

  sensorMsg("INITIALIZATION", "AHT 20 Sensor...");  //Lcd initialization message

  if (aht20.begin() == false)
  {
    Serial.println("AHT20 not detected. Please check wiring. Freezing.");
    while (1);
  }
  Serial.println("AHT20 acknowledged.");

  delay(850);
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
void lcdShow(float temp, float humidity, float pressure, float CO, float CO2, 
              float pm1, float pm25, float pm10, float altitude) {
            //Temperature and Humidity ROW 1
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("TMP=" + String(temp) +"C");
            lcd.setCursor(11,0);
            lcd.print("HMD=" + String(humidity));

            //Pressure ROW 2
            // lcd.setCursor(0,1);
            // lcd.print("PRESS=" + String(pressure) + "Pa");
            lcd.setCursor(0,1);
            lcd.print("PM2.5=" + String((int)pm25));
            lcd.setCursor(10,1);
            lcd.print("PM10=" + String((int)pm10));

            //PM 1.0 and CO ROW 3
            lcd.setCursor(0,2);
            lcd.print("PM1=" + String((int)(pm1)));
            lcd.setCursor(8,2);
            lcd.print("CO=" + String(CO) + "ppm");

            //CO2 and maybe NH4 ROW 4
            lcd.setCursor(0,3);
            lcd.print("CO2=" + String(CO2) + "ppm");
            lcd.setCursor(14,3);
            lcd.print("H=" + String(int(round(altitude))) + "m");


            if(fileSuccess){
              lcd.setCursor(19,1);
              lcd.print("Y");
            } else {
              lcd.setCursor(19,1);
              lcd.print("N");
            }
            //delay(550);
            // lcd.clear();

}


//For the particulate matter sensor
boolean readPMSdata(Stream *s) {
  if (! s->available()) {
    return false;
  }

  // Read a byte at a time until we get to the special '0x42' start-byte
  if (s->peek() != 0x42) {
    s->read();
    return false;
  }

  // Now read all 32 bytes
  if (s->available() < 32) {
    return false;
  }

  uint8_t buffer[32];
  uint16_t sum = 0;
  s->readBytes(buffer, 32);

  // get checksum ready
  for (uint8_t i = 0; i < 30; i++) {
    sum += buffer[i];
  }

  /* debugging
    for (uint8_t i=2; i<32; i++) {
    Serial.print("0x"); Serial.print(buffer[i], HEX); Serial.print(", ");
    }
    Serial.println();
  */

  // The data comes in endian'd, this solves it so it works on all platforms
  uint16_t buffer_u16[15];
  for (uint8_t i = 0; i < 15; i++) {
    buffer_u16[i] = buffer[2 + i * 2 + 1];
    buffer_u16[i] += (buffer[2 + i * 2] << 8);
  }

  // put it into a nice struct :)
  memcpy((void *)&data, (void *)buffer_u16, 30);

  if (sum != data.checksum) {
    Serial.println("Checksum failure");
    return false;
  }
  // success!
  return true;
}
