#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup()
{
	lcd.begin();
  
	Serial.begin(115200);
  
}

void loop()
{
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


  // lcd.setCursor(0,0);
  // lcd.print("----- STUDENTS -----");
  // delay(850);
  // lcd.setCursor(0,1);
  // lcd.print("KWESI MANU EGHAN");
  // lcd.setCursor(0,2);
  // delay(1000);
  // lcd.print("EMMANUEL OPOKU");
  // lcd.setCursor(0,3);
  // delay(1000);
  // lcd.print("AGYEI KWAKU DARKO");
  // delay(3000);
  // lcd.clear();


  // lcd.setCursor(0,0);
  // lcd.print("--- SUPERVISORS ----");
  // delay(850);
  // lcd.setCursor(0,1);
  // lcd.print("PROF. F. K AMPONG");
  // lcd.setCursor(0,2);
  // delay(1000);
  // lcd.print("DR. J.N.A ARYEE");
  // delay(6500);
  // lcd.clear();
}










