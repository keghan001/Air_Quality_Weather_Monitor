#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

#define kRecvPin 26
IRrecv irrecv(kRecvPin);
decode_results cmd;
String myComm;



void(* resetSystem) (void) = 0;  // declare reset fuction at address 0



void setup() {
  Serial.begin(115200);
  Serial.println("I've restarted!!!");
  irrecv.enableIRIn();  // Start the receiver

}

void loop() {
  remoteCheck();
}


void remoteCheck(){
  if (irrecv.decode(&cmd)) {
    if (cmd.value == 0xFFA25D){
      Serial.println("Device restarting\n");
      delay(100);
      resetSystem(); //call reset
    }
    else if (cmd.value == 0xFF629D){
      Serial.println("Device restarting\n");
      delay(100);
      resetSystem(); //call reset
    }
    
    irrecv.resume();  // Receive the next value
  }
  delay(100);
}

