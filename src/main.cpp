#include <ESP8266WiFi.h>
#include "config.h"

#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
#define LEDS_0_PIN 4
#define LEDS_1_PIN 14
#define LEDS_0_NUM 64
#define LEDS_1_NUM 64
#define MAX_BRIGHTNESS 128

CRGB leds0[LEDS_0_NUM];
CRGB leds1[LEDS_1_NUM];


#define FLASH_BUTTON_PIN 0
#define INTERNAL_LED 2  // the tiny blue led on the esp8285
#define LED_ON LOW
#define LED_OFF HIGH

uint8_t state = 1;
#define MAX_STATES 5
uint8_t globalRed;
uint8_t globalGreen;
uint8_t globalBlue;

void waitForWifi() {
  WiFi.hostname(HOSTNAME);
  WiFi.mode(WIFI_STA);
  do {
    Serial.println("Connecting...");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    delay(4000);
  } while (WiFi.status() != WL_CONNECTED);
}

void checkWifi() {
  bool wasDisconnected = false;
  if (WiFi.status() != WL_CONNECTED) {
    wasDisconnected = true;
    Serial.println("Waiting for wifi ...");
    waitForWifi();
  }

  if (wasDisconnected) {
    Serial.println("Connected.");
  }
}

void setColor(CRGB leds[], uint8_t r, uint8_t g, uint8_t b, uint16_t len) {
  for (int x = 0; x < len; x++) {
    leds[x] = CRGB(r, g, b);
  }
}

void updateState() {
  switch (state) {
    case 0:
      globalRed = 0;
      globalGreen = 0;
      globalBlue = 0;
      break;
    case 2:
      globalRed = 255;
      globalGreen = 0;
      globalBlue = 0;
      break;
    case 3:;
      globalRed = 0;
      globalGreen = 255;
      globalBlue = 0;
      break;
    case 4:
      globalRed = 0;
      globalGreen = 0;
      globalBlue = 255;
      break;
    case 1:
    default:
      globalRed = 255;
      globalGreen = 255;
      globalBlue = 255;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(INTERNAL_LED, OUTPUT);
  pinMode(FLASH_BUTTON_PIN, INPUT);

  updateState();
  setColor(leds0, globalRed, globalGreen, globalBlue, LEDS_0_NUM);
  setColor(leds1, globalRed, globalGreen, globalBlue, LEDS_1_NUM);
  LEDS.addLeds<WS2812, LEDS_0_PIN, GRB>(leds0, LEDS_0_NUM);
  LEDS.addLeds<WS2812, LEDS_1_PIN, GRB>(leds1, LEDS_1_NUM);
  LEDS.setBrightness(MAX_BRIGHTNESS);
  LEDS.show();
}

void loop() {
  checkWifi();
  uint8_t button = digitalRead(FLASH_BUTTON_PIN);

  if (button == LOW) {
    digitalWrite(INTERNAL_LED, LED_ON);
    state++;
    state = state > MAX_STATES ? 0 : state;
    updateState();
    setColor(leds0, globalRed, globalGreen, globalBlue, LEDS_0_NUM);
    setColor(leds1, globalRed, globalGreen, globalBlue, LEDS_1_NUM);
    LEDS.show();
    delay(500); // debounce / rate limit
  }

  delay(50);
  digitalWrite(INTERNAL_LED, LED_OFF);
}
