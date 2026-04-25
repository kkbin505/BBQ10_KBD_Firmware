#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>

// Power states
enum PowerState {
  POWER_STATE_ACTIVE,      // Full operation
  POWER_STATE_IDLE,        // Reduced polling rate
  POWER_STATE_CONNECTED,   // Connected but low activity
  POWER_STATE_SLEEP,       // Deep sleep (disconnected)
  POWER_STATE_DEEP_SLEEP   // Ultra low power
};

// Sleep configuration
#define IDLE_TIMEOUT_MS 30000        // 30 seconds to go idle
#define CONNECTED_IDLE_TIMEOUT_MS 120000  // 2 minutes on idle state
#define SLEEP_TIMEOUT_MS 600000      // 10 minutes to deep sleep
#define KEYBOARD_WAKE_PIN 13         // GPIO pin for wake-up

/**
 * Power Management System
 * Handles battery optimization through smart sleep/wake cycling
 */
class PowerManager {
public:
  PowerManager();
  
  // Initialization
  void init();
  
  // Power State Management
  void setConnectionStatus(bool connected);
  void recordActivity();
  void update();
  
  // Sleep Control
  void enterSleep();
  void exitSleep();
  void enterDeepSleep();
  
  // Status queries
  PowerState getPowerState();
  bool isAsleep();
  uint32_t getIdleTime();
  float getBatteryPercentage();
  
  // Configuration
  void setIdleTimeout(uint32_t ms);
  void setSleepTimeout(uint32_t ms);
  
private:
  PowerState currentState;
  uint32_t lastActivityTime;
  uint32_t lastStateChangeTime;
  bool isConnected;
  
  uint32_t idleTimeout;
  uint32_t sleepTimeout;
  uint32_t connectedIdleTimeout;
  
  void updatePowerState();
  void applyPowerState(PowerState newState);
  uint32_t calculatePollingRate();
  float readBatteryVoltage();
};

extern PowerManager powerManager;

#endif // POWER_MANAGER_H
