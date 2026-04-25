#include "ble_profile_manager.h"

// Global instance
BleProfileManager profileManager;

BleProfileManager::BleProfileManager() 
  : activeProfileIndex(0), lastSwitchTime(0) {
  memset(profiles, 0, sizeof(profiles));
}

void BleProfileManager::initProfiles() {
  // Initialize EEPROM if needed
  if (EEPROM.read(EEPROM_MAGIC) != (EEPROM_MAGIC_VALUE & 0xFF)) {
    // EEPROM not initialized, create default profiles
    EEPROM.write(EEPROM_MAGIC, EEPROM_MAGIC_VALUE & 0xFF);
    
    // Initialize empty profiles
    for (uint8_t i = 0; i < MAX_PROFILES; i++) {
      clearProfile(i);
    }
    
    // Set first profile as active
    EEPROM.write(EEPROM_ACTIVE_PROFILE, 0);
    EEPROM.commit();
    
    Serial.println("[BLE] EEPROM initialized with default profiles");
  }
  
  loadProfilesFromEEPROM();
}

void BleProfileManager::loadProfilesFromEEPROM() {
  activeProfileIndex = EEPROM.read(EEPROM_ACTIVE_PROFILE);
  
  if (activeProfileIndex >= MAX_PROFILES) {
    activeProfileIndex = 0;
  }
  
  for (uint8_t i = 0; i < MAX_PROFILES; i++) {
    uint32_t offset = EEPROM_PROFILES_BASE + (i * EEPROM_PROFILE_SIZE);
    
    // Read profile name
    for (uint8_t j = 0; j < 32; j++) {
      profiles[i].name[j] = EEPROM.read(offset + j);
    }
    // Ensure name is null-terminated
    profiles[i].name[31] = '\0';
    
    // Validate name string - if first character is not printable ASCII, clear the profile
    if (profiles[i].name[0] < 32 || profiles[i].name[0] > 126) {
      memset(profiles[i].name, 0, 32);
      memset(profiles[i].address, 0, 6);
    }
    
    // Read BLE address
    for (uint8_t j = 0; j < 6; j++) {
      profiles[i].address[j] = EEPROM.read(offset + 32 + j);
    }
    
    // Read other data
    profiles[i].isActive = (i == activeProfileIndex);
    profiles[i].isConnected = false; // Reset connection status on boot
    profiles[i].lastConnectTime = 0;
    profiles[i].batteryLevel = 100;
    profiles[i].connectionStrength = 0;
  }
  
  Serial.printf("[BLE] Loaded profiles from EEPROM (active: %d)\n", activeProfileIndex);
}

void BleProfileManager::saveProfilesToEEPROM() {
  EEPROM.write(EEPROM_ACTIVE_PROFILE, activeProfileIndex);
  
  for (uint8_t i = 0; i < MAX_PROFILES; i++) {
    uint32_t offset = EEPROM_PROFILES_BASE + (i * EEPROM_PROFILE_SIZE);
    
    // Write profile name
    for (uint8_t j = 0; j < 32; j++) {
      EEPROM.write(offset + j, profiles[i].name[j]);
    }
    
    // Write BLE address
    for (uint8_t j = 0; j < 6; j++) {
      EEPROM.write(offset + 32 + j, profiles[i].address[j]);
    }
  }
  
  EEPROM.commit();
  Serial.printf("[BLE] Saved profiles to EEPROM (active: %d)\n", activeProfileIndex);
}

DeviceProfile* BleProfileManager::getProfile(uint8_t index) {
  if (index < MAX_PROFILES) {
    return &profiles[index];
  }
  return nullptr;
}

DeviceProfile* BleProfileManager::getActiveProfile() {
  return &profiles[activeProfileIndex];
}

uint8_t BleProfileManager::getActiveProfileIndex() {
  return activeProfileIndex;
}

void BleProfileManager::createProfile(uint8_t index, const char* name, const uint8_t* address) {
  if (index >= MAX_PROFILES) return;
  
  strncpy(profiles[index].name, name, 31);
  profiles[index].name[31] = '\0';
  
  if (address) {
    memcpy(profiles[index].address, address, 6);
  }
  
  profiles[index].isConnected = false;
  profiles[index].lastConnectTime = 0;
  profiles[index].batteryLevel = 100;
  profiles[index].connectionStrength = 0;
  
  saveProfilesToEEPROM();
  
  Serial.printf("[BLE] Created profile %d: %s\n", index, name);
}

void BleProfileManager::deleteProfile(uint8_t index) {
  if (index >= MAX_PROFILES) return;
  
  clearProfile(index);
  
  // If we deleted the active profile, switch to first available
  if (index == activeProfileIndex) {
    activeProfileIndex = 0;
  }
  
  saveProfilesToEEPROM();
  Serial.printf("[BLE] Deleted profile %d\n", index);
}

void BleProfileManager::switchProfile(uint8_t index) {
  if (index >= MAX_PROFILES || !isProfileValid(index)) {
    Serial.printf("[BLE] Cannot switch to invalid profile %d\n", index);
    return;
  }
  
  // Check for rapid switching (debounce)
  if (millis() - lastSwitchTime < 500) {
    return;
  }
  
  activeProfileIndex = index;
  lastSwitchTime = millis();
  
  // Update all profiles' active status
  for (uint8_t i = 0; i < MAX_PROFILES; i++) {
    profiles[i].isActive = (i == activeProfileIndex);
  }
  
  profiles[index].lastConnectTime = millis();
  
  saveProfilesToEEPROM();
  
  Serial.printf("[BLE] Switched to profile %d: %s\n", index, profiles[index].name);
}

uint8_t BleProfileManager::getProfileCount() {
  uint8_t count = 0;
  for (uint8_t i = 0; i < MAX_PROFILES; i++) {
    if (isProfileValid(i)) {
      count++;
    }
  }
  return count;
}

bool BleProfileManager::isProfileValid(uint8_t index) {
  if (index >= MAX_PROFILES) return false;
  // A profile is valid if it has a name
  return (profiles[index].name[0] != '\0');
}

void BleProfileManager::updateConnectionStatus(uint8_t index, bool connected) {
  if (index >= MAX_PROFILES) return;
  
  profiles[index].isConnected = connected;
  if (connected) {
    profiles[index].lastConnectTime = millis();
  }
  
  Serial.printf("[BLE] Profile %d (%s) connection: %s\n", 
                index, profiles[index].name, connected ? "CONNECTED" : "DISCONNECTED");
}

void BleProfileManager::updateSignalStrength(int8_t rssi) {
  // Convert RSSI to signal strength indicator (0-100)
  // Typical range: -100 to -30 dBm
  uint8_t strength = 0;
  if (rssi >= -30) {
    strength = 100;
  } else if (rssi <= -100) {
    strength = 0;
  } else {
    strength = (rssi + 100) * 100 / 70;
  }
  
  profiles[activeProfileIndex].connectionStrength = strength;
}

void BleProfileManager::updateBatteryLevel(uint8_t level) {
  if (level <= 100) {
    profiles[activeProfileIndex].batteryLevel = level;
  }
}

void BleProfileManager::printProfileInfo(uint8_t index) {
  if (index >= MAX_PROFILES || !isProfileValid(index)) {
    Serial.printf("Profile %d is not configured\n", index);
    return;
  }
  
  const DeviceProfile* profile = &profiles[index];
  Serial.printf("\n--- Profile %d: %s ---\n", index, profile->name);
  Serial.printf("Active: %s\n", profile->isActive ? "YES" : "NO");
  Serial.printf("Connected: %s\n", profile->isConnected ? "YES" : "NO");
  Serial.printf("Battery: %u%%\n", profile->batteryLevel);
  Serial.printf("Signal: %u%%\n", profile->connectionStrength);
  Serial.printf("Last Connect: %lu seconds ago\n", 
                (millis() - profile->lastConnectTime) / 1000);
}

void BleProfileManager::printAllProfiles() {
  Serial.println("\n=== BLE Device Profiles ===");
  for (uint8_t i = 0; i < MAX_PROFILES; i++) {
    printProfileInfo(i);
  }
}

void BleProfileManager::clearProfile(uint8_t index) {
  if (index >= MAX_PROFILES) return;
  
  memset(&profiles[index], 0, sizeof(DeviceProfile));
}
