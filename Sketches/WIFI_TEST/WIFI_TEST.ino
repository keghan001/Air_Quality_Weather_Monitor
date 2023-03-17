//8. WiFi declarations
#include "WiFi.h"
#include "secrets.h"
const char* ssid = SECRET_SSID;     //  your network SSID (name)
const char* pass = SECRET_PASS;  // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status 




void setup() {
  Serial.begin(115200);
  initWireless();

  printCurrentNet();
  printWifiData();

}

void loop() {
    // check the network connection once every 10 seconds:
  delay(5000);
  printCurrentNet();

}


void initWireless(){
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  // Connect to WPA/WPA2 network:
  status = WiFi.begin(ssid, pass);
      // wait 10 seconds for connection:
    delay(5000);
  }

  // you're connected now, so print out the data:
  Serial.print("You're connected to the network " +  String(ssid));
}



void printWifiData() {
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);

}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  // byte bssid[6];
  // WiFi.BSSID(bssid);
  // Serial.print("BSSID: ");
  // Serial.print(bssid[5], HEX);
  // Serial.print(":");
  // Serial.print(bssid[4], HEX);
  // Serial.print(":");
  // Serial.print(bssid[3], HEX);
  // Serial.print(":");
  // Serial.print(bssid[2], HEX);
  // Serial.print(":");
  // Serial.print(bssid[1], HEX);
  // Serial.print(":");
  // Serial.println(bssid[0], HEX);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // // print the encryption type:
  // byte encryption = WiFi.encryptionType();
  // Serial.print("Encryption Type:");
  // Serial.println(encryption, HEX);
  // Serial.println();
}
