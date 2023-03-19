
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

#define kRecvPin 26
IRrecv irrecv(kRecvPin);
decode_results cmd;
String myComm;



void setup() {
  Serial.begin(115200);
  irrecv.enableIRIn();  // Start the receiver

}

void loop() {
  remoteMap();
}


//Mapping out the buttons on the remote
void remoteMap(){
  if (irrecv.decode(&cmd)) {
    if (cmd.value == 0xFFA25D){
      myComm = "pwrOn";
      Serial.println(myComm);
    }
    else if (cmd.value == 0xFF629D){
      myComm = "pwrOff";
      Serial.println(myComm);
    }
    if (cmd.value == 0xFFE21D){
      myComm = "pwrOn+";
      Serial.println(myComm);
    }
    else if (cmd.value == 0xFF6897){
      myComm = "0";
      Serial.println(myComm);
    }
    else if (cmd.value == 0xFF30CF){
      myComm = "1";
      Serial.println(myComm);
    }
    else if (cmd.value == 0xFF18E7){
      myComm = "2";
      Serial.println(myComm);
    }
    else if (cmd.value == 0xFF7A85){
      myComm = "3";
      Serial.println(myComm);
    }
    else if (cmd.value == 0xFF10EF){
      myComm = "4";
      Serial.println(myComm);
    }
    else if (cmd.value == 0xFF38C7){
      myComm = "5";
      Serial.println(myComm);
    }
    else if (cmd.value == 0xFF5AA5){
      myComm = "6";
      Serial.println(myComm);
    }
    else if (cmd.value == 0xFF42BD){
      myComm = "7";
      Serial.println(myComm);
    }
    else if (cmd.value == 0xFF4AB5){
      myComm = "8";
      Serial.println(myComm);
    }
    else if (cmd.value == 0xFF52AD){
      myComm = "9";
      Serial.println(myComm);
    }
    else if (cmd.value == 0xFFA857){
      myComm = "plus";
      Serial.println(myComm);
    }
    else if (cmd.value == 0xFFE01F){
      myComm = "minus";
      Serial.println(myComm);
    }
    if (cmd.value == 0xFF906F){
      myComm = "EQ";
      Serial.println(myComm);
    }

    irrecv.resume();  // Receive the next value
  }
  delay(100);
}
