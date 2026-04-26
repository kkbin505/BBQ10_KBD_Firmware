#include "StatusLED.h"
#include "WebManager.h"
#include <Arduino.h>
#include <BleKeyboard.h>

extern BleKeyboard Keyboard;

#define LED_PIN 48
#define NUM_PIXELS 1

Adafruit_NeoPixel StatusLED::_pixel(NUM_PIXELS, LED_PIN, NEO_RGB + NEO_KHZ800);

void StatusLED::begin() {
  _pixel.begin();
  _pixel.setBrightness(20); // Set moderate brightness to avoid blinding
  _pixel.show();
}

void StatusLED::setColor(uint8_t r, uint8_t g, uint8_t b) {
  _pixel.setPixelColor(0, _pixel.Color(r, g, b));
  _pixel.show();
}

void StatusLED::update() {
  static uint32_t lastColor = 0;
  static bool stableBleState = false;
  static bool lastRawState = false;
  static unsigned long stateChangeTime = 0;

  uint32_t currentColor;

  if (WebManager::isConfigMode()) {
    // WiFi Config Mode active - Blue
    currentColor = _pixel.Color(0, 0, 255);
  } else {
    // Asymmetrical debounce to hide pairing drops:
    // 500ms to show connected, 3000ms to show disconnected.
    bool rawState = Keyboard.isConnected();
    if (rawState != lastRawState) {
      lastRawState = rawState;
      stateChangeTime = millis();
    }

    // unsigned long currentDebounce = rawState ? 500 : 3000;

    // // Only update stable state after it's been consistent
    // if (millis() - stateChangeTime >= currentDebounce) {
    //     stableBleState = lastRawState;
    // }
    stableBleState = lastRawState;
    if (stableBleState) {
      // BLE Connected - Green (R/G swapped for this hardware)
      currentColor = _pixel.Color(255, 0, 0);
    } else {
      // BLE Disconnected - Red (R/G swapped for this hardware)
      currentColor = _pixel.Color(0, 255, 0);
    }
  }

  if (currentColor != lastColor) {
    _pixel.setPixelColor(0, currentColor);
    _pixel.show();
    lastColor = currentColor;
  }
}
