#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include "config.h"

#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
#define LEDS_0_PIN 4
#define LEDS_1_PIN 14
#define LEDS_0_NUM 50
#define LEDS_1_NUM 50
#define MAX_BRIGHTNESS 128
#define DIST 6
#define OFFSET 5

CRGB leds0[LEDS_0_NUM];
CRGB leds1[LEDS_1_NUM];

#define NUM_BARS 7
uint8_t bars0[] = {4, 11, 17, 23, 30, 36, 42};
uint8_t bars1[] = {4, 10, 16, 22, 29, 35, 41};

#define FLASH_BUTTON_PIN 0
#define INTERNAL_LED 2  // the tiny blue led on the esp8285
#define LED_ON LOW
#define LED_OFF HIGH

uint8_t state = 1;
#define MAX_STATES 4
uint8_t globalRed;
uint8_t globalGreen;
uint8_t globalBlue;

ESP8266WebServer server(80);

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

void setStrandColors(CRGB leds[], uint8_t bars[], uint16_t num, uint8_t r, uint8_t g, uint8_t b, boolean bar) {
  for (int x = 0; x < num; x++) {
    for (int i = 0; i < NUM_BARS; i++) {
      if (bar) {
        if (abs(x - bars[i]) <= 0) {
          // on a bar
          leds[x] = CRGB(r, g, b);
        }
      } else {
        if (abs(x - bars[i]) > 0) {
          // not on a bar
          leds[x] = CRGB(r, g, b);
        }
      }
    }
  }
}

void setStrand0(uint8_t r, uint8_t g, uint8_t b, boolean bar) {
  setStrandColors(leds0, bars0, LEDS_0_NUM, r, g, b, bar);
};

void setStrand1(uint8_t r, uint8_t g, uint8_t b, boolean bar) {
  setStrandColors(leds1, bars1, LEDS_1_NUM, r, g, b, bar);
};

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
  LEDS.addLeds<WS2812, LEDS_0_PIN, RGB>(leds0, LEDS_0_NUM);
  LEDS.addLeds<WS2812, LEDS_1_PIN, RGB>(leds1, LEDS_1_NUM);
  LEDS.setBrightness(MAX_BRIGHTNESS);
  LEDS.show();

  server.on("/brightness", [&]() {
    int v = atoi(server.arg("value").c_str());
    if (v >= 0 && v <= 255) {
      Serial.print("Setting brightness to ");
      Serial.println(v);
      LEDS.setBrightness(v);
      LEDS.show();
      server.send(200, "text/plain", String("Set brightness to ") + String(v));
    }
    server.send(300, "text/plain", String("Error. Input 0 - 255"));
  });

  server.on("/toggle_0", [&]() {
    int v = atoi(server.arg("value").c_str());
    if (v >= 0 && v < LEDS_0_NUM) {
      Serial.print("Toggling led0 value");
      Serial.println(v);

      if (leds0[v].r + leds0[v].g + leds0[v].b == 0) {
        leds0[v] = CRGB(globalRed, globalGreen, globalBlue);
      } else {
        leds0[v] = CRGB(0, 0, 0);
      }

      LEDS.show();
      server.send(200, "text/plain", String("Set value to ") + String(v));
    }
    server.send(300, "text/plain", String("Input error"));
  });

  server.on("/toggle_1", [&]() {
    int v = atoi(server.arg("value").c_str());
    if (v >= 0 && v < LEDS_1_NUM) {
      Serial.print("Toggling led1 value");
      Serial.println(v);

      if (leds1[v].r + leds1[v].g + leds1[v].b == 0) {
        leds1[v] = CRGB(globalRed, globalGreen, globalBlue);
      } else {
        leds1[v] = CRGB(0, 0, 0);
      }

      LEDS.show();
      server.send(200, "text/plain", String("Set value to ") + String(v));
    }
    server.send(300, "text/plain", String("Input error"));
  });

  server.begin();
}

void loop() {
  checkWifi();
  server.handleClient();

  uint8_t button = digitalRead(FLASH_BUTTON_PIN);

  if (button == LOW) {
    digitalWrite(INTERNAL_LED, LED_ON);
    state++;
    state = state > MAX_STATES ? 0 : state;
    Serial.println(state);
    Serial.println(globalRed);
    Serial.println(globalGreen);
    Serial.println(globalBlue);

    updateState();
    setStrand0(64, 64, 64, false); // wall
    setStrand0(globalRed, globalGreen, globalBlue, true); // bar

    setStrand1(64, 64, 64, false); // wall
    setStrand1(globalRed, globalGreen, globalBlue, true); // bar

    LEDS.show();
    delay(500);  // debounce / rate limit
  }

  delay(5);
  digitalWrite(INTERNAL_LED, LED_OFF);
}
