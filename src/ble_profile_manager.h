#ifndef BLE_PROFILE_MANAGER_H
#define BLE_PROFILE_MANAGER_H

#include <Arduino.h>
#include <EEPROM.h>
#include <string.h>

// Maximum number of device profiles
#define MAX_PROFILES 3

// EEPROM addresses
#define EEPROM_MAGIC 0
#define EEPROM_ACTIVE_PROFILE 4
#define EEPROM_PROFILES_BASE 8
#define EEPROM_PROFILE_SIZE 64
#define EEPROM_TOTAL_SIZE (EEPROM_PROFILES_BASE + (MAX_PROFILES * EEPROM_PROFILE_SIZE))

// Magic number to detect valid EEPROM data
#define EEPROM_MAGIC_VALUE 0xDEADBEEF

/**
 * Device Profile Structure
 * Stores information about a bonded BLE device
 */
struct DeviceProfile {
  char name[32];           // Device name (Phone A, Phone B, Laptop)
  uint8_t address[6];      // BLE MAC address
  bool isActive;          // Currently active profile
  bool isConnected;       // Current connection status
  uint32_t lastConnectTime; // Timestamp of last connection
  uint32_t batteryLevel;  // Last known battery level (0-100)
  uint8_t connectionStrength; // RSSI signal strength indicator
};

class BleProfileManager {
public:
  BleProfileManager();
  
  // Profile Management
  void initProfiles();
  void loadProfilesFromEEPROM();
  void saveProfilesToEEPROM();
  
  // Profile Access
  DeviceProfile* getProfile(uint8_t index);
  DeviceProfile* getActiveProfile();
  uint8_t getActiveProfileIndex();
  
  // Profile Operations
  void createProfile(uint8_t index, const char* name, const uint8_t* address);
  void deleteProfile(uint8_t index);
  void switchProfile(uint8_t index);
  
  // Profile Information
  uint8_t getProfileCount();
  bool isProfileValid(uint8_t index);
  void printProfileInfo(uint8_t index);
  void printAllProfiles();
  
  // Update Profile Status
  void updateConnectionStatus(uint8_t index, bool connected);
  void updateSignalStrength(int8_t rssi);
  void updateBatteryLevel(uint8_t level);
  
private:
  DeviceProfile profiles[MAX_PROFILES];
  uint8_t activeProfileIndex;
  uint32_t lastSwitchTime;
  
  void clearProfile(uint8_t index);
};

extern BleProfileManager profileManager;

#endif // BLE_PROFILE_MANAGER_H
