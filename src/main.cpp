#include <Arduino.h>
#include <ctype.h>
#include <BleKeyboard.h>
#include <NimBLEDevice.h>
#include <EEPROM.h>
#include "ble_profile_manager.h"
#include "power_manager.h"
#include "device_switcher.h"

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

// ============ DEBUGGING MODE: USB HID DISABLED ============
// Disabled USB HID for serial debugging of BLE switching
// #include <USB.h>
// #define KeyReport UsbKeyReport
// #include <USBHIDKeyboard.h>
// #undef KeyReport

BleKeyboard Keyboard("BBQ10-BLE2", "BBQ10", 100);
// USBHIDKeyboard UsbKeyboard;  // DISABLED FOR DEBUGGING

constexpr uint8_t rows[] = {6, 7, 8, 9, 10, 11, 12};
constexpr uint8_t cols[] = {1, 2, 3, 4, 5};

constexpr size_t ROW_COUNT = sizeof(rows) / sizeof(rows[0]);
constexpr size_t COL_COUNT = sizeof(cols) / sizeof(cols[0]);

bool keys[COL_COUNT][ROW_COUNT] = {};
bool lastValue[COL_COUNT][ROW_COUNT] = {};
bool changedValue[COL_COUNT][ROW_COUNT] = {};

constexpr unsigned long LONG_PRESS_MS = 280;

constexpr size_t D_COL = 1;
constexpr size_t D_ROW = 2;
constexpr size_t H_COL = 3;
constexpr size_t H_ROW = 1;
constexpr size_t L_COL = 4;
constexpr size_t L_ROW = 1;
constexpr size_t BACKSPACE_COL = 4;
constexpr size_t BACKSPACE_ROW = 3;
// NOTE: LEFT_SHIFT was at [0][4] which conflicts with ALT - removed this constant
// Only RIGHT_SHIFT is used
constexpr size_t RIGHT_SHIFT_COL = 2;
constexpr size_t RIGHT_SHIFT_ROW = 3;
constexpr size_t CTRL_COL = 1;
constexpr size_t CTRL_ROW = 6;
constexpr size_t LAYER2_COL = 0;
constexpr size_t LAYER2_ROW = 2;
constexpr size_t ALT_COL = 0;
constexpr size_t ALT_ROW = 4;
// [col][row]
const char keyboard[COL_COUNT][ROW_COUNT] = {
  {'q', 'w', '\0', 'a', '\0', ' ', '\0'},
  {'e', 's', 'd', 'p', 'x', 'z', '\0'},
  {'r', 'g', 't', '\0', 'v', 'c', 'f'},
  {'u', 'h', 'y', '\0', 'b', 'n', 'j'},
  {'o', 'l', 'i', '\0', '$', 'm', 'k'},
};

const char keyboardSymbol[COL_COUNT][ROW_COUNT] = {
  {'#', '1', '\0', '*', '\0', '\0', '0'},
  {'2', '4', '5', '@', '8', '7', '\0'},
  {'3', '/', '(', '\0', '?', '9', '6'},
  {'_', ':', ')', '\0', '!', ',', ';'},
  {'+', '"', '-', '\0', '\0', '.', '\''},
};

// Special keys: TAB, ENTER, ESC, etc.
// 0 means no special key at that position
const uint8_t keyboardSpecial[COL_COUNT][ROW_COUNT] = {
  {0, 0, 0, 0, 0, 0, 0},              // col 0: ALT is handled separately, not here
  {0, 0, 0, 0, 0, 0, 0},              // col 1
  {0, 0, 0, 0, 0, 0, 0},              // col 2
  {0, 0, 0, 0, 0, 0, 0},              // col 3
  {0, 0, 0, 0, 0, 0, 0},              // col 4
};

bool ctrlModifierActive = false;
bool shiftModifierActive = false;
bool shiftUsedWithOtherKey = false;

// Layer 2 toggle state (SYM key)
bool layer2Toggle = false;
bool layer2LastPressed = false;

// Device switching - ALT + 1/2/3 to switch devices
// We'll use a special key combination for device switching
bool altModifierActive = false;
const size_t SWITCH_DEVICE_COL = 0;  // ALT position
const size_t SWITCH_DEVICE_ROW = 4;

// Track previous BLE connection state for state changes
bool prevBleConnected = false;
// BLE connection state filtering - add debounce for stability
unsigned long bleStateChangeTime = 0;
unsigned long bleCurrentStateDetectTime = 0;
bool bleCurrentState = false;
const unsigned long BLE_STATE_DEBOUNCE_MS = 500;  // 500ms debounce to filter noise

// Track if device switching was handled to prevent key repeat
bool deviceSwitchHandled = false;
size_t switchKeyCol = 0;
size_t switchKeyRow = 0;

struct LongPressKey {
  size_t col;
  size_t row;
  char baseShortOutput;
  char baseLongOutput;
  char layer2ShortOutput;
  char layer2LongOutput;
  bool tracking;
  bool longSent;
  bool layer2AtPress;
  unsigned long pressStart;
};

LongPressKey longPressKeys[] = {
  {D_COL, D_ROW, 'd', 'f', '5', '6', false, false, false, 0},
  {H_COL, H_ROW, 'h', 'j', ':', ';', false, false, false, 0},
  {L_COL, L_ROW, 'l', 'k', '"', '\'', false, false, false, 0},
};

bool keyPressed(size_t colIndex, size_t rowIndex) {
  return changedValue[colIndex][rowIndex] && keys[colIndex][rowIndex];
}

bool keyActive(size_t colIndex, size_t rowIndex) {
  return keys[colIndex][rowIndex];
}

bool isPrintableKey(size_t colIndex, size_t rowIndex) {
  return keyboard[colIndex][rowIndex] != '\0' || keyboardSymbol[colIndex][rowIndex] != '\0';
}

bool isLayer2Active() {
  return layer2Toggle;
}

void updateLayer2Toggle() {
  const bool symPressed = keyActive(LAYER2_COL, LAYER2_ROW);
  if (symPressed && !layer2LastPressed) {
    // SYM key just pressed — toggle Layer 2
    layer2Toggle = !layer2Toggle;
    Serial.printf("[DEBUG] SYM pressed - Layer 2 now: %s\n", layer2Toggle ? "ON" : "OFF");
  }
  layer2LastPressed = symPressed;
}

bool isLongPressManagedKey(size_t colIndex, size_t rowIndex) {
  for (size_t i = 0; i < (sizeof(longPressKeys) / sizeof(longPressKeys[0])); i++) {
    if (longPressKeys[i].col == colIndex && longPressKeys[i].row == rowIndex) {
      return true;
    }
  }
  return false;
}

void updateCtrlModifier() {
  const bool ctrlHeldNow = keyActive(CTRL_COL, CTRL_ROW);

  if (ctrlHeldNow && !ctrlModifierActive) {
    if (Keyboard.isConnected()) {
      Keyboard.press(KEY_LEFT_CTRL);
    }
    // UsbKeyboard.press(KEY_LEFT_CTRL);  // DISABLED FOR DEBUGGING
    ctrlModifierActive = true;
  } else if (!ctrlHeldNow && ctrlModifierActive) {
    if (Keyboard.isConnected()) {
      Keyboard.release(KEY_LEFT_CTRL);
    }
    // UsbKeyboard.release(KEY_LEFT_CTRL);  // DISABLED FOR DEBUGGING
    ctrlModifierActive = false;
  }
}

void updateShiftModifier() {
  // Bug fix: Only check RIGHT_SHIFT - ALT key ([0][4]) is handled separately in updateAltModifier()
  const bool shiftHeldNow = keyActive(RIGHT_SHIFT_COL, RIGHT_SHIFT_ROW);

  if (shiftHeldNow && !shiftModifierActive) {
    if (Keyboard.isConnected()) {
      Keyboard.press(KEY_LEFT_SHIFT);
    }
    // UsbKeyboard.press(KEY_LEFT_SHIFT);  // DISABLED FOR DEBUGGING
    shiftModifierActive = true;
    shiftUsedWithOtherKey = false;
  } else if (!shiftHeldNow && shiftModifierActive) {
    if (Keyboard.isConnected()) {
      Keyboard.release(KEY_LEFT_SHIFT);
    }
    // UsbKeyboard.release(KEY_LEFT_SHIFT);  // DISABLED FOR DEBUGGING

    // If Shift was pressed alone, emit a standalone Shift tap for IME toggle behavior.
    if (!shiftUsedWithOtherKey) {
      if (Keyboard.isConnected()) {
        Keyboard.write(KEY_LEFT_SHIFT);
      }
      // UsbKeyboard.write(KEY_LEFT_SHIFT);  // DISABLED FOR DEBUGGING
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

  const uint8_t keycode = static_cast<uint8_t>(output);
  if (Keyboard.isConnected()) {
    Keyboard.write(keycode);
  }
  // UsbKeyboard.write(keycode);  // DISABLED FOR DEBUGGING
}

void emitSpecialKey(uint8_t keycode) {
  if (shiftModifierActive && keycode != KEY_LEFT_SHIFT && keycode != KEY_RIGHT_SHIFT) {
    shiftUsedWithOtherKey = true;
  }

  if (Keyboard.isConnected()) {
    Keyboard.write(keycode);
  }
  // UsbKeyboard.write(keycode);  // DISABLED FOR DEBUGGING
}

void updateAltModifier() {
  const bool altHeldNow = keyActive(ALT_COL, ALT_ROW);

  if (altHeldNow && !altModifierActive) {
    altModifierActive = true;
    
    // Layer 2 + ALT = TAB
    if (isLayer2Active()) {
      Serial.println("[DEBUG] ALT pressed with Layer 2 active - sending TAB");
      emitSpecialKey(KEY_TAB);
      layer2Toggle = false;  // Exit Layer 2 after TAB
    } else {
      Serial.println("[DEBUG] ALT pressed");
    }
  } else if (!altHeldNow && altModifierActive) {
    altModifierActive = false;
    Serial.println("[DEBUG] ALT released");
  }
}

void handleDeviceSwitching() {
  static unsigned long lastSwitchTime = 0;
  const unsigned long now = millis();
  
  // Bug fix: Add debounce - prevent rapid re-switching (1 second cooldown)
  if (now - lastSwitchTime < 1000) {
    return;
  }
  
  // Check for ALT+ W/E/R to switch devices
  if (altModifierActive) {
    Serial.printf("[DEBUG] ALT+SYM active, checking for device switch keys...\n");
    
    // Debug: print key states
    Serial.printf("[DEBUG] Key states: [0][0]=%d [0][1]=%d [1][0]=%d [2][0]=%d\n",
                  keys[0][0], keys[0][1], keys[1][0], keys[2][0]);
    
    if (keyPressed(0, 1)) {  // ALT + W = device 0
      Serial.println("[DEBUG] MATCHED: [0][1] W key pressed");
      lastSwitchTime = now;
      deviceSwitcher.switchToProfile(0);
      powerManager.recordActivity();
      deviceSwitchHandled = true;
      switchKeyCol = 0;
      switchKeyRow = 1;
      Serial.println("[SWITCH] Device 0 selected via ALT+SYM+W");
    } else if (keyPressed(1, 0)) {  // ALT + E = device 1
      Serial.println("[DEBUG] MATCHED: [1][0] E key pressed");
      lastSwitchTime = now;
      deviceSwitcher.switchToProfile(1);
      powerManager.recordActivity();
      deviceSwitchHandled = true;
      switchKeyCol = 1;
      switchKeyRow = 0;
      Serial.println("[SWITCH] Device 1 selected via ALT+SYM+E");
    } else if (keyPressed(2, 0)) {  // ALT + R = device 2
      Serial.println("[DEBUG] MATCHED: [2][0] R key pressed");
      lastSwitchTime = now;
      deviceSwitcher.switchToProfile(2);
      powerManager.recordActivity();
      deviceSwitchHandled = true;
      switchKeyCol = 2;
      switchKeyRow = 0;
      layer2Toggle = false;  // Bug fix: Reset Layer 2 after switching
      Serial.println("[SWITCH] Device 2 selected via ALT+SYM+R");
    }
  }
}

void updateBleConnectionStatus() {
  bool currentBleConnected = Keyboard.isConnected();
  const unsigned long now = millis();
  
  // Detect potential state change
  if (currentBleConnected != bleCurrentState) {
    // State changed, start debounce timer
    if (bleCurrentStateDetectTime == 0) {
      bleCurrentStateDetectTime = now;
    }
    
    // Wait for debounce period to confirm state change
    if (now - bleCurrentStateDetectTime >= BLE_STATE_DEBOUNCE_MS) {
      // State confirmed after debounce
      if (currentBleConnected != prevBleConnected) {
        prevBleConnected = currentBleConnected;
        bleStateChangeTime = now;
        
        uint8_t activeProfile = profileManager.getActiveProfileIndex();
        profileManager.updateConnectionStatus(activeProfile, currentBleConnected);
        powerManager.setConnectionStatus(currentBleConnected);
        
        if (currentBleConnected) {
          Serial.printf("[BLE] STABLE: Connected to profile %d: %s\n", 
                        activeProfile, 
                        profileManager.getProfile(activeProfile)->name);
        } else {
          Serial.println("[BLE] STABLE: Disconnected");
        }
      }
      bleCurrentState = currentBleConnected;
      bleCurrentStateDetectTime = 0;
    }
  } else {
    // State stable, reset debounce timer
    bleCurrentStateDetectTime = 0;
  }
}

void processLongPressFallbacks() {
  const unsigned long now = millis();
  const bool layer2Active = isLayer2Active();

  for (size_t i = 0; i < (sizeof(longPressKeys) / sizeof(longPressKeys[0])); i++) {
    LongPressKey &key = longPressKeys[i];
    const bool active = keyActive(key.col, key.row);

    if (active) {
      if (!key.tracking) {
        key.tracking = true;
        key.longSent = false;
        key.layer2AtPress = layer2Active;
        key.pressStart = now;
      } else if (!key.longSent && (now - key.pressStart) >= LONG_PRESS_MS) {
        const char longOutput = key.layer2AtPress ? key.layer2LongOutput : key.baseLongOutput;
        emitKey(longOutput);
        key.longSent = true;
      }
    } else if (key.tracking) {
      if (!key.longSent) {
        const char shortOutput = key.layer2AtPress ? key.layer2ShortOutput : key.baseShortOutput;
        emitKey(shortOutput);
      }
      key.tracking = false;
      key.longSent = false;
      key.layer2AtPress = false;
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
      changedValue[colIndex][rowIndex] = (lastValue[colIndex][rowIndex] != buttonPressed);
      lastValue[colIndex][rowIndex] = buttonPressed;

      pinMode(curRow, INPUT);
    }

    pinMode(curCol, INPUT);
  }
}

void printMatrix() {
  const bool layer2Active = isLayer2Active();

  for (size_t rowIndex = 0; rowIndex < ROW_COUNT; rowIndex++) {
    for (size_t colIndex = 0; colIndex < COL_COUNT; colIndex++) {
      if (!keyPressed(colIndex, rowIndex)) {
        continue;
      }

      // Skip keys that were handled by device switching
      if (deviceSwitchHandled && colIndex == switchKeyCol && rowIndex == switchKeyRow) {
        continue;
      }

      // Skip ALT and SYM modifier keys from being emitted as characters
      if ((colIndex == ALT_COL && rowIndex == ALT_ROW) || 
          (colIndex == LAYER2_COL && rowIndex == LAYER2_ROW)) {
        continue;
      }

      if (colIndex == BACKSPACE_COL && rowIndex == BACKSPACE_ROW) {
        emitSpecialKey(KEY_BACKSPACE);
        continue;
      }

      // Check for special keys in Layer 2 (e.g., SYM + ALT → TAB)
      if (layer2Active && keyboardSpecial[colIndex][rowIndex] != 0) {
        emitSpecialKey(keyboardSpecial[colIndex][rowIndex]);
        continue;
      }

      if (!isPrintableKey(colIndex, rowIndex)) {
        continue;
      }

      if (isLongPressManagedKey(colIndex, rowIndex)) {
        continue;
      }

      char output = keyboard[colIndex][rowIndex];
      if (layer2Active && keyboardSymbol[colIndex][rowIndex] != '\0') {
        output = keyboardSymbol[colIndex][rowIndex];
      }

      if (output == '\0') {
        continue;
      }

      emitKey(output);
    }
  }
  
  // Reset device switch flag at the end of matrix processing
  deviceSwitchHandled = false;
}

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  delay(500);
  Serial.println("\n[SYSTEM] BBQ10 BLE Keyboard - DEBUG MODE (USB HID DISABLED)");
  
  // USB.begin();  // DISABLED FOR DEBUGGING
  // UsbKeyboard.begin();  // DISABLED FOR DEBUGGING

  Keyboard.setDelay(8);
  Keyboard.begin();

  // Initialize EEPROM
  EEPROM.begin(EEPROM_TOTAL_SIZE);
  delay(100);
  
  // Initialize profile manager
  profileManager.initProfiles();
  Serial.println("[SYSTEM] Profile manager initialized");
  
  // Create default device profiles if not already created
  Serial.println("[SYSTEM] Setting up default device profiles...");
  if (profileManager.getProfileCount() == 0) {
    // Create three default profiles
    // Note: BLE addresses are placeholder values - they will be updated when devices are bonded
    uint8_t addr0[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x00};
    uint8_t addr1[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x01};
    uint8_t addr2[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x02};
    
    profileManager.createProfile(0, "Phone A", addr0);
    profileManager.createProfile(1, "Phone B", addr1);
    profileManager.createProfile(2, "Laptop", addr2);
    
    Serial.println("[SYSTEM] Created 3 default device profiles");
  }
  
  // Initialize power manager
  powerManager.init();
  Serial.println("[SYSTEM] Power manager initialized");
  
  // Initialize device switcher
  deviceSwitcher.init(SWITCH_MODE_BUTTON);
  Serial.println("[SYSTEM] Device switcher initialized");
  
  // Print profile information
  profileManager.printAllProfiles();

  delay(200);

  for (size_t i = 0; i < ROW_COUNT; i++) {
    pinMode(rows[i], INPUT);
  }

  for (size_t i = 0; i < COL_COUNT; i++) {
    pinMode(cols[i], INPUT_PULLUP);
  }
  
  Serial.println("[SYSTEM] Setup complete\n");
}

void loop() {
  readMatrix();
  
  // Bug fix: Use flag to properly break out of nested loops
  bool activityDetected = false;
  for (size_t col = 0; col < COL_COUNT && !activityDetected; col++) {
    for (size_t row = 0; row < ROW_COUNT && !activityDetected; row++) {
      if (keyPressed(col, row) || keyActive(col, row)) {
        powerManager.recordActivity();
        activityDetected = true;
      }
    }
  }
  
  // Update modifiers and BLE status
  updateLayer2Toggle();
  updateShiftModifier();  // Bug fix: Now only checks RIGHT_SHIFT
  updateCtrlModifier();
  updateAltModifier();
  updateBleConnectionStatus();
  
  // Handle device switching (ALT + SYM + number)
  handleDeviceSwitching();
  
  // Process keys
  processLongPressFallbacks();
  printMatrix();

  // Enter key at [3][3]
  if (keyPressed(3, 3)) {
    emitSpecialKey(KEY_RETURN);
  }
  
  // Update power and device management systems
  powerManager.update();
  deviceSwitcher.update();

  delay(10);
}
