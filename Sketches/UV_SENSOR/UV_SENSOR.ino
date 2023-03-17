int uvPin = 4;   // select the input for the UV sensor pin A1 on UNO
float uvIndex;

void setup() {
  // declare the ledPin as an OUTPUT:
  Serial.begin(115200);
  pinMode(uvPin, INPUT);
}

void loop() {
  float sensorValue = analogRead(uvPin);

  Serial.println("Analog: " + String(sensorValue));
  uvIndex = readUV(uvPin);
  Serial.println("UV Index: " + String(uvIndex));
  // stop the program for for <sensorValue> milliseconds:
  delay(1000);
}



int readUV(int sensorPin){
  int index = 0;
  int sensorValue = 0;
  
  sensorValue = analogRead(sensorPin);  //connect UV sensor to Analog 0   
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

  return index;
}
