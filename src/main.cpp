#include <Arduino.h>
#include <ctype.h>
#include <BleKeyboard.h>
#include <NimBLEDevice.h>

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

BleKeyboard Keyboard("BBQ10-BLE2", "BBQ10", 100);
USBHIDKeyboard UsbKeyboard;

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
constexpr size_t LEFT_SHIFT_COL = 0;
constexpr size_t LEFT_SHIFT_ROW = 4;
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
  {0, 0, 0, 0, KEY_TAB, 0, 0},        // col 0: ALT at [0][4] produces TAB in Layer 2
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
    UsbKeyboard.press(KEY_LEFT_CTRL);
    ctrlModifierActive = true;
  } else if (!ctrlHeldNow && ctrlModifierActive) {
    if (Keyboard.isConnected()) {
      Keyboard.release(KEY_LEFT_CTRL);
    }
    UsbKeyboard.release(KEY_LEFT_CTRL);
    ctrlModifierActive = false;
  }
}

void updateShiftModifier() {
  const bool shiftHeldNow = keyActive(LEFT_SHIFT_COL, LEFT_SHIFT_ROW)
                         || keyActive(RIGHT_SHIFT_COL, RIGHT_SHIFT_ROW);

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

    // If Shift was pressed alone, emit a standalone Shift tap for IME toggle behavior.
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

  const uint8_t keycode = static_cast<uint8_t>(output);
  if (Keyboard.isConnected()) {
    Keyboard.write(keycode);
  }
  UsbKeyboard.write(keycode);
}

void emitSpecialKey(uint8_t keycode) {
  if (shiftModifierActive && keycode != KEY_LEFT_SHIFT && keycode != KEY_RIGHT_SHIFT) {
    shiftUsedWithOtherKey = true;
  }

  if (Keyboard.isConnected()) {
    Keyboard.write(keycode);
  }
  UsbKeyboard.write(keycode);
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
}

void setup() {
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
}

void loop() {
  readMatrix();
  updateLayer2Toggle();
  updateShiftModifier();
  updateCtrlModifier();
  processLongPressFallbacks();
  printMatrix();

  // Enter key at [3][3]
  if (keyPressed(3, 3)) {
    emitSpecialKey(KEY_RETURN);
  }

  delay(10);
}
