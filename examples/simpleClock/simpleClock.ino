#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <espRTC.h>

const char* ssid     = "your-ssid";
const char* password = "your-pass";

// Instantiate the espRTC Class
espRTC myRTC = espRTC("time.nist.gov", -5, true); // epsRTC(NTP_Server, GMT Offset, Day_Light_Savings)

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());  

  // Initialize the RTC after WiFi connection has been established
  myRTC.begin();
  
}

void loop() {
  
  myRTC.update(); // Periodically call RTC Update so it can check if it needs to sync with the NTP Server

  // Simple Print of the current Date & Time every 10 Seconds:
  Serial.print(myRTC.getMonth());
  Serial.print("/");
  Serial.print(myRTC.getDay());
  Serial.print("/");
  Serial.println(myRTC.getYear());
  
  Serial.print(myRTC.getHour());
  Serial.print(":");
  Serial.print(myRTC.getMinute());
  Serial.print(":");
  Serial.println(myRTC.getSecond());
  delay(10000);
  
}


