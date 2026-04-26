#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <vector>

constexpr size_t CM_ROW_COUNT = 7;
constexpr size_t CM_COL_COUNT = 5;

// Define LongPressKey struct here so it can be used globally
struct LongPressKey {
  size_t col;
  size_t row;
  char baseShortOutput;
  char baseLongOutput;
  char layer2ShortOutput;
  char layer2LongOutput;
  bool tracking;
  bool longSent;
  uint8_t layerAtPress;
  unsigned long pressStart;
};

class ConfigManager {
public:
    static void begin();
    static void loadConfig();
    static void saveConfig();

    static void setDefaults();

    // Configuration state
    static char keyboard[CM_COL_COUNT][CM_ROW_COUNT];
    static char keyboardSymbol[CM_COL_COUNT][CM_ROW_COUNT];
    static uint8_t keyboardLayer3[CM_COL_COUNT][CM_ROW_COUNT];
    static uint8_t keyboardSpecial[CM_COL_COUNT][CM_ROW_COUNT];
    static std::vector<LongPressKey> longPressKeys;
};
