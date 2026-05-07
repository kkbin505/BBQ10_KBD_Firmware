#include <Arduino.h>
#include <BleKeyboard.h>
#include <NimBLEDevice.h>
#include <ctype.h>
#include <esp_pm.h>
#include <esp_sleep.h>

#ifdef HID_SUBCLASS_NONE
#undef HID_SUBCLASS_NONE
#endif
#ifdef HID_SUBCLASS_BOOT_INTERFACE
#undef HID_SUBCLASS_BOOT_INTERFACE
#endif
#ifdef HID_PROTOCOL_NONE
#undef HID_PROTOCOL_NONE
#endif
#ifdef HID_PROTOCOL_KEYBOARD
#undef HID_PROTOCOL_KEYBOARD
#endif
#ifdef HID_PROTOCOL_MOUSE
#undef HID_PROTOCOL_MOUSE
#endif

#include <USB.h>
#define KeyReport UsbKeyReport
#include <USBHIDKeyboard.h>
#undef KeyReport

#include "ConfigManager.h"
#include "StatusLED.h"
#include "WebManager.h"

BleKeyboard Keyboard("BBQ10KBD", "BBQ10KBD", 100);
USBHIDKeyboard UsbKeyboard;

constexpr uint8_t rows[] = {6, 7, 8, 9, 10, 11, 12};
constexpr uint8_t cols[] = {1, 2, 3, 4, 5};

constexpr size_t ROW_COUNT = sizeof(rows) / sizeof(rows[0]);
constexpr size_t COL_COUNT = sizeof(cols) / sizeof(cols[0]);

bool keys[COL_COUNT][ROW_COUNT] = {};
bool lastValue[COL_COUNT][ROW_COUNT] = {};
bool changedValue[COL_COUNT][ROW_COUNT] = {};

constexpr unsigned long LONG_PRESS_MS = 280;

bool isEmittingLongPress = false;

constexpr size_t D_COL = 1;
constexpr size_t D_ROW = 2;
constexpr size_t H_COL = 3;
constexpr size_t H_ROW = 1;
constexpr size_t L_COL = 4;
constexpr size_t L_ROW = 1;
constexpr size_t BACKSPACE_COL = 4;
constexpr size_t BACKSPACE_ROW = 3;
constexpr size_t LEFT_SHIFT_COL = 1;
constexpr size_t LEFT_SHIFT_ROW = 6;
constexpr size_t RIGHT_SHIFT_COL = 2;
constexpr size_t RIGHT_SHIFT_ROW = 3;
// constexpr size_t CTRL_COL = 1;
// constexpr size_t CTRL_ROW = 6;
constexpr size_t LAYER2_COL = 0;
constexpr size_t LAYER2_ROW = 2;
constexpr size_t ALT_COL = 0;
constexpr size_t ALT_ROW = 4;

// Power management configuration
constexpr unsigned long INACTIVITY_TIMEOUT_MS = 10 * 60 * 1000; // 10 minutes
constexpr unsigned long BLE_DISCONNECT_GRACE_PERIOD_MS =
    30 * 1000; // 30 seconds
unsigned long lastActivityTime = 0;
unsigned long lastBleDisconnectTime = 0;
bool isInLightSleep = false;
bool wasConnectedPreviously = true;

// Config loaded from LittleFS

bool ctrlModifierActive = false;
bool shiftModifierActive = false;
bool shiftUsedWithOtherKey = false;
bool cmdModifierActive = false;
bool cmdUsedWithOtherKey = false;

// Layer 2 toggle state (SYM key)
// Layer state: 0=Base, 1=Symbols, 2=Navigation
uint8_t currentLayer = 0;
bool layer2LastPressed = false;
unsigned long lastSymPressTime = 0;
constexpr unsigned long DOUBLE_TAP_MS = 300;

// Forward declarations for power management functions
void updateLastActivity();
bool isBleConnected();
void updateBleDisconnectTimer();
bool shouldEnterSleep();
void enterLightSleep();

bool keyPressed(size_t colIndex, size_t rowIndex) {
  return changedValue[colIndex][rowIndex] && keys[colIndex][rowIndex];
}

bool keyActive(size_t colIndex, size_t rowIndex) {
  return keys[colIndex][rowIndex];
}

bool isPrintableKey(size_t colIndex, size_t rowIndex) {
  return ConfigManager::keyboard[colIndex][rowIndex] != '\0' ||
         ConfigManager::keyboardSymbol[colIndex][rowIndex] != '\0' ||
         ConfigManager::keyboardLayer3[colIndex][rowIndex] != '\0';
}

bool isLayer2Active() { return currentLayer == 1; }
bool isLayer3Active() { return currentLayer == 2; }

void updateLayer2Toggle() {
  const bool symPressed = keyActive(LAYER2_COL, LAYER2_ROW);
  const unsigned long now = millis();

  if (symPressed && !layer2LastPressed) {
    const bool isQuickTap = (now - lastSymPressTime < DOUBLE_TAP_MS);

    switch (currentLayer) {
    case 1: // From Layer 2 (Symbols)
      currentLayer = isQuickTap ? 2 : 0;
      break;
    case 2: // From Layer 3 (Navigation)
      currentLayer = 0;
      break;
    default: // From Layer 1 (Base)
      currentLayer = isQuickTap ? 2 : 1;
      break;
    }
    lastSymPressTime = now;
  }

  // 10 second hold for config mode
  if (symPressed && (now - lastSymPressTime > wifi_enable_press_time)) {
    WebManager::startConfigMode();
  }

  layer2LastPressed = symPressed;
}

bool isLongPressManagedKey(size_t colIndex, size_t rowIndex) {
  // If Layer 3 has an override for this key, don't treat it as long-press
  // managed so that printMatrix can handle it directly.
  if (currentLayer == 2 &&
      ConfigManager::keyboardLayer3[colIndex][rowIndex] != 0) {
    return false;
  }

  for (size_t i = 0; i < ConfigManager::longPressKeys.size(); i++) {
    if (ConfigManager::longPressKeys[i].col == colIndex &&
        ConfigManager::longPressKeys[i].row == rowIndex) {
      return true;
    }
  }
  return false;
}

// void updateCtrlModifier() {
//   const bool ctrlHeldNow = keyActive(CTRL_COL, CTRL_ROW);

//   if (ctrlHeldNow && !ctrlModifierActive) {
//     if (Keyboard.isConnected()) {
//       Keyboard.press(KEY_LEFT_CTRL);
//     }
//     UsbKeyboard.press(KEY_LEFT_CTRL);
//     ctrlModifierActive = true;
//   } else if (!ctrlHeldNow && ctrlModifierActive) {
//     if (Keyboard.isConnected()) {
//       Keyboard.release(KEY_LEFT_CTRL);
//     }
//     UsbKeyboard.release(KEY_LEFT_CTRL);
//     ctrlModifierActive = false;
//   }
// }

void updateCmdModifier() {
  const bool cmdHeldNow = keyActive(ALT_COL, ALT_ROW);

  if (cmdHeldNow && !cmdModifierActive) {
    if (Keyboard.isConnected()) {
      Keyboard.press(KEY_LEFT_CTRL);
    }
    UsbKeyboard.press(KEY_LEFT_CTRL);
    cmdModifierActive = true;
    cmdUsedWithOtherKey = false;
  } else if (!cmdHeldNow && cmdModifierActive) {
    if (Keyboard.isConnected()) {
      Keyboard.release(KEY_LEFT_CTRL);
    }
    UsbKeyboard.release(KEY_LEFT_CTRL);
    cmdModifierActive = false;
  }
}

void updateShiftModifier() {
  const bool shiftHeldNow = keyActive(LEFT_SHIFT_COL, LEFT_SHIFT_ROW) ||
                            keyActive(RIGHT_SHIFT_COL, RIGHT_SHIFT_ROW);

  if (shiftHeldNow && !shiftModifierActive) {
    if (Keyboard.isConnected()) {
      Keyboard.press(KEY_LEFT_SHIFT);
    }
    UsbKeyboard.press(KEY_LEFT_SHIFT);
    shiftModifierActive = true;
    shiftUsedWithOtherKey = false;
  } else if (!shiftHeldNow && shiftModifierActive) {
    if (Keyboard.isConnected()) {
      Keyboard.release(KEY_LEFT_SHIFT);
    }
    UsbKeyboard.release(KEY_LEFT_SHIFT);

    // If Shift was pressed alone, emit a standalone Shift tap for IME toggle
    // behavior.
    if (!shiftUsedWithOtherKey) {
      if (Keyboard.isConnected()) {
        Keyboard.write(KEY_LEFT_SHIFT);
      }
      UsbKeyboard.write(KEY_LEFT_SHIFT);
    }

    shiftModifierActive = false;
    shiftUsedWithOtherKey = false;
  }
}

void emitKey(char output) {
  if (output == '\0') {
    return;
  }

  if (shiftModifierActive) {
    shiftUsedWithOtherKey = true;
  }

  if (cmdModifierActive) {
    cmdUsedWithOtherKey = true;
    // Cancel long-press for ALT if it's being used as a modifier for another
    // key
    if (!isEmittingLongPress) {
      for (size_t i = 0; i < ConfigManager::longPressKeys.size(); i++) {
        if (ConfigManager::longPressKeys[i].col == ALT_COL &&
            ConfigManager::longPressKeys[i].row == ALT_ROW) {
          ConfigManager::longPressKeys[i].tracking = false;
          break;
        }
      }
    }
  }

  const uint8_t keycode = static_cast<uint8_t>(output);
  if (Keyboard.isConnected()) {
    Keyboard.write(keycode);
  }
  UsbKeyboard.write(keycode);
}

void emitSpecialKey(uint8_t keycode) {
  if (shiftModifierActive && keycode != KEY_LEFT_SHIFT &&
      keycode != KEY_RIGHT_SHIFT) {
    shiftUsedWithOtherKey = true;
  }

  if (cmdModifierActive && keycode != KEY_LEFT_CTRL) {
    cmdUsedWithOtherKey = true;
    // Cancel long-press for ALT if it's being used as a modifier for another
    // key
    if (!isEmittingLongPress) {
      for (size_t i = 0; i < ConfigManager::longPressKeys.size(); i++) {
        if (ConfigManager::longPressKeys[i].col == ALT_COL &&
            ConfigManager::longPressKeys[i].row == ALT_ROW) {
          ConfigManager::longPressKeys[i].tracking = false;
          break;
        }
      }
    }
  }

  if (Keyboard.isConnected()) {
    Keyboard.write(keycode);
  }
  UsbKeyboard.write(keycode);
}

void processLongPressFallbacks() {
  const unsigned long now = millis();
  const uint8_t activeLayer = currentLayer;

  for (size_t i = 0; i < ConfigManager::longPressKeys.size(); i++) {
    LongPressKey &key = ConfigManager::longPressKeys[i];
    const bool active = keyActive(key.col, key.row);

    if (active) {
      if (!key.tracking) {
        key.tracking = true;
        key.longSent = false;
        key.layerAtPress = activeLayer;
        key.pressStart = now;
      } else if (!key.longSent && (now - key.pressStart) >= LONG_PRESS_MS) {
        // Special case for ALT: release Command before sending Tab to avoid
        // Cmd+Tab
        if (key.col == ALT_COL && key.row == ALT_ROW) {
          if (Keyboard.isConnected()) {
            Keyboard.release(KEY_LEFT_CTRL);
          }
          UsbKeyboard.release(KEY_LEFT_CTRL);
        }

        // Keys in Layer 3 are handled by printMatrix(), so skip here
        if (key.layerAtPress != 2) {
          const char longOutput = (key.layerAtPress == 1) ? key.layer2LongOutput
                                                          : key.baseLongOutput;
          isEmittingLongPress = true;
          emitKey(longOutput);
          if (key.col == 4 && key.row == 5) { // 'm' key long press -> ".."
            emitKey('.');
          }
          isEmittingLongPress = false;
        }
        key.longSent = true; // Mark as sent to prevent re-triggering on release
      }
    } else if (key.tracking) {
      if (!key.longSent && key.layerAtPress != 2) { // Skip for Layer 3
        const char shortOutput = (key.layerAtPress == 1) ? key.layer2ShortOutput
                                                         : key.baseShortOutput;
        emitKey(shortOutput);
      }
      key.tracking = false;
      key.longSent = false;
      key.layerAtPress = 0;

      // Update activity on key release
      updateLastActivity();
    }
  }
}

void readMatrix() {
  for (size_t colIndex = 0; colIndex < COL_COUNT; colIndex++) {
    const uint8_t curCol = cols[colIndex];

    pinMode(curCol, OUTPUT);
    digitalWrite(curCol, LOW);

    for (size_t rowIndex = 0; rowIndex < ROW_COUNT; rowIndex++) {
      const uint8_t curRow = rows[rowIndex];
      pinMode(curRow, INPUT_PULLUP);
      delayMicroseconds(200);

      const bool buttonPressed = (digitalRead(curRow) == LOW);
      keys[colIndex][rowIndex] = buttonPressed;
      changedValue[colIndex][rowIndex] =
          (lastValue[colIndex][rowIndex] != buttonPressed);
      lastValue[colIndex][rowIndex] = buttonPressed;

      pinMode(curRow, INPUT);
    }

    pinMode(curCol, INPUT);
  }
}

void printMatrix() {
  const uint8_t activeLayer = currentLayer;

  for (size_t rowIndex = 0; rowIndex < ROW_COUNT; rowIndex++) {
    for (size_t colIndex = 0; colIndex < COL_COUNT; colIndex++) {
      if (!keyPressed(colIndex, rowIndex)) {
        continue;
      }

      // Update last activity time when any key is pressed
      updateLastActivity();

      if (colIndex == BACKSPACE_COL && rowIndex == BACKSPACE_ROW) {
        emitSpecialKey(KEY_BACKSPACE);
        continue;
      }

      if (ConfigManager::keyboardSpecial[colIndex][rowIndex] != 0) {
        emitSpecialKey(ConfigManager::keyboardSpecial[colIndex][rowIndex]);
        continue;
      }

      // Check for Navigation keys in Layer 3
      if (activeLayer == 2 &&
          ConfigManager::keyboardLayer3[colIndex][rowIndex] != 0) {
        emitSpecialKey(ConfigManager::keyboardLayer3[colIndex][rowIndex]);
        continue;
      }

      if (!isPrintableKey(colIndex, rowIndex)) {
        continue;
      }

      if (isLongPressManagedKey(colIndex, rowIndex)) {
        continue;
      }

      char output = ConfigManager::keyboard[colIndex][rowIndex];
      if (activeLayer == 1 &&
          ConfigManager::keyboardSymbol[colIndex][rowIndex] != '\0') {
        output = ConfigManager::keyboardSymbol[colIndex][rowIndex];
      } else if (activeLayer == 2 &&
                 ConfigManager::keyboardLayer3[colIndex][rowIndex] != '\0') {
        output = (char)ConfigManager::keyboardLayer3[colIndex][rowIndex];
      }

      if (output == '\0') {
        continue;
      }

      emitKey(output);
    }
  }
}

// Power Management Functions
void updateLastActivity() {
  lastActivityTime = millis();
  isInLightSleep = false;
}

bool isBleConnected() { return Keyboard.isConnected(); }

void updateBleDisconnectTimer() {
  const bool currentConnected = isBleConnected();

  if (wasConnectedPreviously && !currentConnected) {
    // BLE just disconnected
    lastBleDisconnectTime = millis();
  }
  wasConnectedPreviously = currentConnected;
}

bool shouldEnterSleep() {
  const unsigned long now = millis();
  const unsigned long inactiveTime = now - lastActivityTime;
  const bool bleConnected = isBleConnected();

  // Must be inactive for at least INACTIVITY_TIMEOUT_MS
  if (inactiveTime < INACTIVITY_TIMEOUT_MS) {
    return false;
  }

  // If BLE is connected, allow sleep
  if (bleConnected) {
    return true;
  }

  // If BLE is disconnected, wait GRACE_PERIOD before sleeping
  const unsigned long disconnectTime = now - lastBleDisconnectTime;
  return disconnectTime >= BLE_DISCONNECT_GRACE_PERIOD_MS;
}

void enterLightSleep() {
  // Configure GPIO wakeup on row pins (pressed = LOW)
  // This allows any key press to wake the device
  // Using GPIO 6 (first row pin) as the wakeup trigger
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_6, 0); // Row 6, trigger on LOW

  isInLightSleep = true;
  esp_light_sleep_start();

  // Code resumes here after wakeup
  updateLastActivity(); // Reset activity timer on wake
}

void setup() {
  ConfigManager::begin();
  WebManager::begin();
  StatusLED::begin();

  // Configure CPU frequency and dynamic power management
  // Set max frequency to 80MHz, min to 20MHz (auto-scales when idle)
  esp_pm_config_esp32s3_t pm_config = {
      .max_freq_mhz = 80, .min_freq_mhz = 20, .light_sleep_enable = true};
  esp_pm_configure(&pm_config);

  USB.begin();
  UsbKeyboard.begin();

  Keyboard.setDelay(8);
  Keyboard.begin();

  // BLE bonds are preserved so the keyboard auto-reconnects to paired hosts.

  delay(200);

  for (size_t i = 0; i < ROW_COUNT; i++) {
    pinMode(rows[i], INPUT);
  }

  for (size_t i = 0; i < COL_COUNT; i++) {
    pinMode(cols[i], INPUT_PULLUP);
  }

  // Initialize activity timer
  lastActivityTime = millis();
}

void loop() {
  StatusLED::update();

  if (WebManager::isConfigMode()) {
    WebManager::handle();
    delay(10);
    return;
  }

  readMatrix();
  updateLayer2Toggle();
  updateShiftModifier();
  // updateCtrlModifier();
  updateCmdModifier();
  processLongPressFallbacks();
  printMatrix();

  // Enter key at [3][3]
  if (keyPressed(3, 3)) {
    emitSpecialKey(KEY_RETURN);
  }

  // Monitor BLE connection status
  updateBleDisconnectTimer();

  // Check if should enter light sleep
  if (shouldEnterSleep()) {
    enterLightSleep();
  }

  delay(10);
}
