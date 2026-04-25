#include "device_switcher.h"
#include <BleKeyboard.h>

// Global instance
DeviceSwitcher deviceSwitcher;

// Forward declaration of BleKeyboard instance from main.cpp
extern BleKeyboard Keyboard;

DeviceSwitcher::DeviceSwitcher()
  : switchMode(SWITCH_MODE_BUTTON),
    currentProfile(0),
    lastSwitchTime(0),
    switchCallback(nullptr),
    modifierHeld(false),
    modifierPressTime(0),
    switchButtonHeld(false),
    switchButtonPressTime(0),
    lastCycleTime(0) {
}

void DeviceSwitcher::init(SwitchMode mode) {
  switchMode = mode;
  currentProfile = profileManager.getActiveProfileIndex();
  Serial.printf("[SWITCH] Device Switcher initialized in mode: %d\n", mode);
}

void DeviceSwitcher::switchToNext() {
  if (!canSwitch()) return;
  
  uint8_t nextProfile = (currentProfile + 1) % MAX_PROFILES;
  
  // Skip invalid profiles
  while (!profileManager.isProfileValid(nextProfile) && nextProfile != currentProfile) {
    nextProfile = (nextProfile + 1) % MAX_PROFILES;
  }
  
  if (nextProfile != currentProfile) {
    switchToProfile(nextProfile);
  }
}

void DeviceSwitcher::switchToPrevious() {
  if (!canSwitch()) return;
  
  uint8_t prevProfile = (currentProfile == 0) ? (MAX_PROFILES - 1) : (currentProfile - 1);
  
  // Skip invalid profiles
  uint8_t attempts = 0;
  while (!profileManager.isProfileValid(prevProfile) && attempts < MAX_PROFILES) {
    prevProfile = (prevProfile == 0) ? (MAX_PROFILES - 1) : (prevProfile - 1);
    attempts++;
  }
  
  if (prevProfile != currentProfile) {
    switchToProfile(prevProfile);
  }
}

void DeviceSwitcher::switchToProfile(uint8_t profileIndex) {
  if (!profileManager.isProfileValid(profileIndex)) {
    Serial.printf("[SWITCH] Cannot switch to invalid profile %d\n", profileIndex);
    return;
  }
  
  if (!canSwitch()) return;
  
  // Only perform switch if changing to a different profile
  if (profileIndex == currentProfile) {
    Serial.printf("[SWITCH] Already on profile %d\n", profileIndex);
    return;
  }
  
  currentProfile = profileIndex;
  lastSwitchTime = millis();
  
  // Update profile manager to save to EEPROM
  profileManager.switchProfile(profileIndex);
  
  // NOTE: Disabled Keyboard.end()/begin() as it causes connection instability
  // Instead, rely on BLE library's internal reconnection mechanism
  // The BLE stack will automatically attempt to connect to the new profile
  Serial.println("[SWITCH] Device switch queued (BLE will reconnect automatically)");
  delay(200);  // Brief delay for BLE stack to process profile change
  
  // Trigger callback
  if (switchCallback) {
    switchCallback(profileIndex);
  }
  
  // Visual feedback
  Serial.printf("[SWITCH] Switched to profile %d: %s - Waiting for BLE reconnection\n", 
                profileIndex, profileManager.getProfile(profileIndex)->name);
}

void DeviceSwitcher::handleKeyPress(size_t colIndex, size_t rowIndex) {
  // This is called when any key is pressed
  // In chord mode, detect modifier + number combinations
  // In button mode, detect dedicated switch button
}

void DeviceSwitcher::handleKeyRelease(size_t colIndex, size_t rowIndex) {
  // This is called when any key is released
}

void DeviceSwitcher::update() {
  // Update switching logic based on current mode
  switch (switchMode) {
    case SWITCH_MODE_BUTTON:
      // Single button mode - handled externally
      break;
      
    case SWITCH_MODE_CHORD:
      // Chord mode - detect modifier + number patterns
      // Timeout handling for released chords
      if (modifierHeld && (millis() - modifierPressTime > 1000)) {
        modifierHeld = false;
      }
      break;
      
    case SWITCH_MODE_SEQUENTIAL:
      // Sequential mode - cycle through devices while button held
      if (switchButtonHeld && (millis() - lastCycleTime > 300)) {
        switchToNext();
        lastCycleTime = millis();
      }
      break;
  }
}

void DeviceSwitcher::setSwitchCallback(SwitchCallback callback) {
  switchCallback = callback;
}

uint8_t DeviceSwitcher::getCurrentProfile() {
  return currentProfile;
}

const char* DeviceSwitcher::getCurrentProfileName() {
  DeviceProfile* profile = profileManager.getProfile(currentProfile);
  if (profile) {
    return profile->name;
  }
  return "Unknown";
}

void DeviceSwitcher::printStatus() {
  Serial.printf("\n=== Device Switcher Status ===\n");
  Serial.printf("Current Profile: %d (%s)\n", currentProfile, getCurrentProfileName());
  Serial.printf("Switch Mode: %d\n", switchMode);
  Serial.printf("All Profiles:\n");
  
  for (uint8_t i = 0; i < MAX_PROFILES; i++) {
    if (profileManager.isProfileValid(i)) {
      DeviceProfile* profile = profileManager.getProfile(i);
      Serial.printf("  %d. %s %s\n", 
                    i, 
                    profile->name,
                    (i == currentProfile) ? "<<< ACTIVE" : "");
    }
  }
}

bool DeviceSwitcher::canSwitch() {
  // Debounce: prevent rapid switching
  return (millis() - lastSwitchTime > 500);
}

void DeviceSwitcher::performSwitch(uint8_t newProfile) {
  switchToProfile(newProfile);
}
