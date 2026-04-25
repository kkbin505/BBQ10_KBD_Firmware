#ifndef DEVICE_SWITCHER_H
#define DEVICE_SWITCHER_H

#include <Arduino.h>
#include "ble_profile_manager.h"

// Device switching modes
enum SwitchMode {
  SWITCH_MODE_BUTTON,      // Single button press cycles through devices
  SWITCH_MODE_CHORD,       // Hold modifier + number (e.g., ALT+1, ALT+2, ALT+3)
  SWITCH_MODE_SEQUENTIAL   // Hold button to cycle through devices
};

// Callback function type for device switch events
typedef void (*SwitchCallback)(uint8_t profileIndex);

/**
 * Device Switcher
 * Handles rapid switching between bonded BLE devices
 */
class DeviceSwitcher {
public:
  DeviceSwitcher();
  
  // Initialization
  void init(SwitchMode mode = SWITCH_MODE_BUTTON);
  
  // Switching
  void switchToNext();
  void switchToPrevious();
  void switchToProfile(uint8_t profileIndex);
  
  // Switch detection from keyboard input
  void handleKeyPress(size_t colIndex, size_t rowIndex);
  void handleKeyRelease(size_t colIndex, size_t rowIndex);
  void update();
  
  // Callbacks
  void setSwitchCallback(SwitchCallback callback);
  
  // Status
  uint8_t getCurrentProfile();
  const char* getCurrentProfileName();
  void printStatus();
  
private:
  SwitchMode switchMode;
  uint8_t currentProfile;
  uint32_t lastSwitchTime;
  SwitchCallback switchCallback;
  
  // For chord mode
  bool modifierHeld;
  uint32_t modifierPressTime;
  
  // For sequential mode
  bool switchButtonHeld;
  uint32_t switchButtonPressTime;
  uint32_t lastCycleTime;
  
  bool canSwitch();
  void performSwitch(uint8_t newProfile);
};

extern DeviceSwitcher deviceSwitcher;

#endif // DEVICE_SWITCHER_H
