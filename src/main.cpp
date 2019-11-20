#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include "config.h"

#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
#define LEDS_0_PIN 4
#define LEDS_1_PIN 14
#define LEDS_0_NUM 46
#define LEDS_1_NUM 46
#define MAX_BRIGHTNESS 128
#define DIST 6
#define OFFSET 5

CRGB leds0[LEDS_0_NUM];
CRGB leds1[LEDS_1_NUM];
uint8_t brightness = MAX_BRIGHTNESS;

#define NUM_BARS 7
uint8_t bars0[] = {4, 11, 17, 23, 30, 36, 42};
uint8_t bars1[] = {4, 10, 16, 22, 29, 35, 41};
uint16_t h[] = {0, 20, 40, 60, 80, 100, 120};
uint8_t hR;  // hue dim value
uint8_t hG;
uint8_t hB;
unsigned long lastChanged = 0;
unsigned long changeInterval = 250;

#define FLASH_BUTTON_PIN 0
#define INTERNAL_LED 2  // the tiny blue led on the esp8285
#define LED_ON LOW
#define LED_OFF HIGH

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

void setLedColorHSV(byte h, byte s, byte v) {
  // this is the algorithm to convert from RGB to HSV
  h = (h * 192) / 256;            // 0..191
  unsigned int i = h / 32;        // We want a value of 0 thru 5
  unsigned int f = (h % 32) * 8;  // 'fractional' part of 'i' 0..248 in jumps

  unsigned int sInv = 255 - s;  // 0 -> 0xff, 0xff -> 0
  unsigned int fInv = 255 - f;  // 0 -> 0xff, 0xff -> 0
  byte pv = v * sInv / 256;     // pv will be in range 0 - 255
  byte qv = v * (256 - s * f / 256) / 256;
  byte tv = v * (256 - s * fInv / 256) / 256;

  switch (i) {
    case 0:
      hR = v;
      hG = tv;
      hB = pv;
      break;
    case 1:
      hR = qv;
      hG = v;
      hB = pv;
      break;
    case 2:
      hR = pv;
      hG = v;
      hB = tv;
      break;
    case 3:
      hR = pv;
      hG = qv;
      hB = v;
      break;
    case 4:
      hR = tv;
      hG = pv;
      hB = v;
      break;
    case 5:
      hR = v;
      hG = pv;
      hB = qv;
      break;
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
          setLedColorHSV(h[i], 255, 255);  // calculate hR,hG,hB...
          leds[x] = CRGB(hR, hG, hB);
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

void setup() {
  Serial.begin(115200);

  pinMode(INTERNAL_LED, OUTPUT);
  pinMode(FLASH_BUTTON_PIN, INPUT);

  LEDS.addLeds<WS2812, LEDS_0_PIN, RGB>(leds0, LEDS_0_NUM);
  LEDS.addLeds<WS2812, LEDS_1_PIN, RGB>(leds1, LEDS_1_NUM);
  LEDS.setBrightness(brightness);
  LEDS.show();

  server.on("/brightness", [&]() {
    int v = atoi(server.arg("value").c_str());
    if (v >= 0 && v <= 255) {
      Serial.print("Setting brightness to ");
      brightness = v;
      Serial.println(v);
      server.send(200, "text/plain", String("Set brightness to ") + String(v));
    }
    server.send(300, "text/plain", String("Error. Input 0 - 255"));
  });

  server.on("/speed", [&]() {
    uint v = atoi(server.arg("value").c_str());
    if (v >= 0 && v < 10000) {
      Serial.print("Set speed ");
      Serial.println(v);

      changeInterval = v;

      server.send(200, "text/plain", String("Set speed to ") + String(v));
    }
    server.send(300, "text/plain", String("Input error"));
  });

  server.begin();
  digitalWrite(INTERNAL_LED, LED_OFF);
}

void loop() {
  checkWifi();
  server.handleClient();

  uint8_t button = digitalRead(FLASH_BUTTON_PIN);
  if (button == LOW) {
    digitalWrite(INTERNAL_LED, LED_ON);
    delay(500);  // debounce / rate limit
    digitalWrite(INTERNAL_LED, LED_OFF);
  }

  if (millis() - lastChanged > changeInterval) {
    for (uint8_t i = 0; i < NUM_BARS; i++) {
      h[i]++;
    }
    setStrand0(32, 32, 32, false);  // wall
    setStrand0(0, 0, 0, true);      // bar

    setStrand1(32, 32, 32, false);  // wall
    setStrand1(0, 0, 0, true);      // bar

    // I borked the last led
    leds1[45] = CRGB(0, 0, 0);
    LEDS.setBrightness(brightness);
    LEDS.show();

    lastChanged = millis();
  }

  delay(5);
}
