#define alertPin 14

void alert(int pin, int reps=1, int waits=1000) {
  for(int i=0; i<reps; i++){
    digitalWrite(pin, HIGH);
    delay(waits);
    digitalWrite(pin, LOW);
    delay(waits);
  }
}

void setup() {
  
  pinMode(alertPin, OUTPUT);
  alert(alertPin, 10, 100);
}

void loop() {
  
}


