#include "power_manager.h"
#include <esp32/clk.h>

// Global instance
PowerManager powerManager;

PowerManager::PowerManager()
  : currentState(POWER_STATE_ACTIVE),
    lastActivityTime(millis()),
    lastStateChangeTime(millis()),
    isConnected(false),
    idleTimeout(IDLE_TIMEOUT_MS),
    sleepTimeout(SLEEP_TIMEOUT_MS),
    connectedIdleTimeout(CONNECTED_IDLE_TIMEOUT_MS) {
}

void PowerManager::init() {
  // Configure wake-up pins
  pinMode(KEYBOARD_WAKE_PIN, INPUT_PULLUP);
  
  // Enable wake-up from GPIO
  esp_sleep_enable_gpio_wakeup();
  gpio_wakeup_enable((gpio_num_t)KEYBOARD_WAKE_PIN, GPIO_INTR_LOW_LEVEL);
  
  Serial.println("[POWER] Power manager initialized");
}

void PowerManager::setConnectionStatus(bool connected) {
  if (isConnected != connected) {
    isConnected = connected;
    lastActivityTime = millis();
    lastStateChangeTime = millis();
    
    // Only log significant state changes to avoid serial spam
    Serial.printf("[POWER] BLE State: %s\n", connected ? "CONNECTED" : "DISCONNECTED");
    
    // Immediately update state based on connection status
    updatePowerState();
  }
}

void PowerManager::recordActivity() {
  lastActivityTime = millis();
  
  // If in sleep mode, wake up on activity
  if (currentState == POWER_STATE_SLEEP || currentState == POWER_STATE_DEEP_SLEEP) {
    exitSleep();
  } else if (currentState != POWER_STATE_ACTIVE) {
    currentState = POWER_STATE_ACTIVE;
    applyPowerState(POWER_STATE_ACTIVE);
  }
}

void PowerManager::update() {
  updatePowerState();
}

void PowerManager::updatePowerState() {
  uint32_t idleTime = millis() - lastActivityTime;
  PowerState newState = POWER_STATE_ACTIVE;
  
  if (!isConnected) {
    // Not connected to any device
    if (idleTime > sleepTimeout) {
      newState = POWER_STATE_DEEP_SLEEP;
    } else if (idleTime > idleTimeout) {
      newState = POWER_STATE_SLEEP;
    } else if (idleTime > connectedIdleTimeout / 2) {
      newState = POWER_STATE_IDLE;
    } else {
      newState = POWER_STATE_ACTIVE;
    }
  } else {
    // Connected to a device
    if (idleTime > sleepTimeout) {
      newState = POWER_STATE_DEEP_SLEEP;
    } else if (idleTime > connectedIdleTimeout) {
      newState = POWER_STATE_SLEEP;
    } else if (idleTime > idleTimeout) {
      newState = POWER_STATE_CONNECTED;
    } else {
      newState = POWER_STATE_ACTIVE;
    }
  }
  
  // Apply new state if changed
  if (newState != currentState) {
    applyPowerState(newState);
    lastStateChangeTime = millis();
  }
}

void PowerManager::applyPowerState(PowerState newState) {
  // Reduce CPU frequency in lower power states
  switch (newState) {
    case POWER_STATE_ACTIVE:
      setCpuFrequencyMhz(240);  // Full speed
      Serial.println("[POWER] State: ACTIVE (240 MHz)");
      break;
      
    case POWER_STATE_IDLE:
      setCpuFrequencyMhz(160);  // Reduced speed
      Serial.println("[POWER] State: IDLE (160 MHz)");
      break;
      
    case POWER_STATE_CONNECTED:
      setCpuFrequencyMhz(80);   // Very low speed
      Serial.println("[POWER] State: CONNECTED (80 MHz)");
      break;
      
    case POWER_STATE_SLEEP:
      setCpuFrequencyMhz(40);   // Minimal speed
      Serial.println("[POWER] State: SLEEP (40 MHz)");
      // Optional: Put Bluetooth to light sleep
      break;
      
    case POWER_STATE_DEEP_SLEEP:
      enterDeepSleep();
      break;
  }
  
  currentState = newState;
}

void PowerManager::enterSleep() {
  Serial.println("[POWER] Entering light sleep...");
  currentState = POWER_STATE_SLEEP;
  
  // Reduce CPU frequency
  setCpuFrequencyMhz(40);
  
  // Optional: Configure light sleep
  // esp_sleep_enable_timer_wakeup(1000000); // Wake after 1 second
}

void PowerManager::exitSleep() {
  Serial.println("[POWER] Exiting sleep...");
  currentState = POWER_STATE_ACTIVE;
  setCpuFrequencyMhz(240);
  lastActivityTime = millis();
}

void PowerManager::enterDeepSleep() {
  Serial.println("[POWER] Entering deep sleep (15 minutes)...");
  
  // Configure wake-up timer (15 minutes)
  esp_sleep_enable_timer_wakeup(15 * 60 * 1000000);
  
  // Configure GPIO wake-up for any key press
  esp_sleep_enable_gpio_wakeup();
  
  // Enter deep sleep
  esp_deep_sleep_start();
}

PowerState PowerManager::getPowerState() {
  return currentState;
}

bool PowerManager::isAsleep() {
  return (currentState == POWER_STATE_SLEEP || currentState == POWER_STATE_DEEP_SLEEP);
}

uint32_t PowerManager::getIdleTime() {
  return millis() - lastActivityTime;
}

float PowerManager::getBatteryPercentage() {
  // Read battery voltage from ADC
  // This is a placeholder - adjust GPIO pin and calibration based on your hardware
  float voltage = readBatteryVoltage();
  
  // Typical lithium battery: 3V = 0%, 4.2V = 100%
  float batteryPercent = (voltage - 3.0f) / 1.2f * 100.0f;
  
  // Clamp to 0-100
  if (batteryPercent < 0) batteryPercent = 0;
  if (batteryPercent > 100) batteryPercent = 100;
  
  return batteryPercent;
}

void PowerManager::setIdleTimeout(uint32_t ms) {
  idleTimeout = ms;
}

void PowerManager::setSleepTimeout(uint32_t ms) {
  sleepTimeout = ms;
}

float PowerManager::readBatteryVoltage() {
  // Read ADC value (adjust ADC pin based on your hardware)
  // Typical setup: ADC on GPIO 34 (ADC1_CH6)
  int rawValue = analogRead(34);
  
  // Convert ADC value to voltage
  // ADC range: 0-4095 for 0-3.3V (with 150mV offset for 0-4.2V battery)
  float voltage = (rawValue / 4095.0f) * 3.3f * 1.28f; // 1.28 is calibration factor
  
  return voltage;
}
