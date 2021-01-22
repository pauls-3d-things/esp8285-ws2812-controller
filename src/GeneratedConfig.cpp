#include "GeneratedConfig.h"
#include <Arduino.h>

uint8_t Config::getId() { return 1; };

uint32_t Config::getSettingsBrightness(DynamicJsonDocument& root) {
  return root["Settings"]["Brightness"];
}