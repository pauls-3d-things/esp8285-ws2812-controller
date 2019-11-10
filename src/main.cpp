#include <ESP8266WiFi.h>

uint8 ledpin = 2;

void setup() {
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  pinMode(ledpin, OUTPUT);
}

void loop() {
  digitalWrite(ledpin, LOW);
  delay(1000);
  digitalWrite(ledpin, HIGH);
  delay(1000);
}
