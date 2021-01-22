#ifndef DNALamp_H
#define DNALamp_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ConfigServerConfig.h>
class Config : public ConfigServerConfig {
    public:
      uint8_t getId();
      uint32_t getSettingsBrightness(DynamicJsonDocument& root);
    };
#endif