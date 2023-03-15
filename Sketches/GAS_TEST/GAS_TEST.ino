//MQ-135 Sensor declarations
#include <MQUnifiedsensor.h>
#define placa "ESP32"
#define Voltage_Resolution 5
#define pin 2 //Analog input 0 of your arduino
#define type "MQ-135" //MQ135
#define ADC_Bit_Resolution 10 // For arduino UNO/MEGA/NANO
#define RatioMQ135CleanAir 3.6//RS / R0 = 3.6 ppm  
//Declare Sensor
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);



void setup() {
  Serial.begin(115200); //Init serial port
  initGasSensor();
}

void loop() {
  MQ135.update(); // Update data, the arduino will read the voltage from the analog pin

  MQ135.setA(605.18); MQ135.setB(-3.937); // Configure the equation to calculate CO concentration value
  float CO = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(110.47); MQ135.setB(-2.862); // Configure the equation to calculate CO2 concentration value
  float CO2 = MQ135.readSensor() + 400; // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(102.2 ); MQ135.setB(-2.473); // Configure the equation to calculate NH4 concentration value
  float NH4 = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

 
  Serial.print("|   "); Serial.print(CO); 
  Serial.print("   |   "); Serial.print(CO2); 
  Serial.print("   |   "); Serial.print(NH4); 
  Serial.println("   |"); 
 

  delay(500); //Sampling frequency
}



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
