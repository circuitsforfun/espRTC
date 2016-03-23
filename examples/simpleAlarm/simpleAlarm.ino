#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <espRTC.h>

const char* ssid     = "your-ssid";
const char* password = "your-pass";

int LEDPin = 12; // LED pin for alarm indicator

// Instantiate the espRTC Class
espRTC myRTC = espRTC("time.nist.gov", -5, true); // epsRTC(NTP_Server, GMT Offset, Day_Light_Savings)

void setup() {
  pinMode(LEDPin, OUTPUT);
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

  // Check alarm time (8:30pm)
  if (myRTC.getHour() == 20 && myRTC.getMinute() == 30)
  {
    // Print and Flash LED for 1 minute
    Serial.println("ALARM!!! It's 8:30 PM!!!");
    digitalWrite(LEDPin, HIGH);
    delay(1000);
    digitalWrite(LEDPin, LOW);
    delay(1000);
  }
  
  myRTC.update(); // Periodically call RTC Update so it can check if it needs to sync with the NTP Server

}


