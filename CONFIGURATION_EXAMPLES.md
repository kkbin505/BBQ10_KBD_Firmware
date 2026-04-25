/**
 * BBQ10 BLE Profile System - Configuration Example
 * 
 * 这个文件展示了如何配置和使用 BLE 配置文件管理系统
 */

#include "ble_profile_manager.h"
#include "power_manager.h"
#include "device_switcher.h"

/**
 * 示例 1: 设置多个设备配置文件
 * 
 * 这个函数展示了如何创建和初始化多个设备配置文件
 */
void example_setup_profiles() {
  Serial.println("\\n=== Setting up Device Profiles ===\\n");
  
  // 方法 1: 自动从 EEPROM 加载（首次运行时创建默认配置）
  profileManager.initProfiles();
  
  // 方法 2: 手动创建配置文件（如果需要重置）
  // 注意：实际的 BLE 地址应该从已配对的设备获取
  uint8_t phoneA_addr[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x01};
  uint8_t phoneB_addr[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x02};
  uint8_t laptop_addr[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x03};
  
  profileManager.createProfile(0, \"Phone A\", phoneA_addr);
  profileManager.createProfile(1, \"Phone B\", phoneB_addr);
  profileManager.createProfile(2, \"Laptop\", laptop_addr);
  
  // 打印所有配置文件信息
  profileManager.printAllProfiles();
}

/**
 * 示例 2: 快速设备切换
 * 
 * 这个函数展示了不同的设备切换方式
 */
void example_device_switching() {
  Serial.println("\\n=== Device Switching Examples ===\\n");
  
  // 方式 1: 切换到下一个设备
  Serial.println(\"Switching to next device...\");
  deviceSwitcher.switchToNext();
  delay(500);
  
  // 方式 2: 切换到指定设备
  Serial.println(\"Switching to profile 1 (Phone B)...\");
  deviceSwitcher.switchToProfile(1);
  delay(500);
  
  // 方式 3: 切换到上一个设备
  Serial.println(\"Switching to previous device...\");
  deviceSwitcher.switchToPrevious();
  delay(500);
  
  // 打印当前状态
  deviceSwitcher.printStatus();
}

/**
 * 示例 3: 电源管理
 * 
 * 这个函数展示了如何使用电源管理系统
 */
void example_power_management() {
  Serial.println(\"\\n=== Power Management Examples ===\\n\");
  
  // 设置连接状态
  Serial.println(\"Simulating BLE connection...\");
  powerManager.setConnectionStatus(true);
  delay(1000);
  
  // 记录活动
  Serial.println(\"Recording user activity...\");
  powerManager.recordActivity();
  
  // 查询电源状态
  Serial.printf(\"Current Power State: %d\\n\", powerManager.getPowerState());
  Serial.printf(\"Idle Time: %lu ms\\n\", powerManager.getIdleTime());
  Serial.printf(\"Battery Level: %.1f%%\\n\", powerManager.getBatteryPercentage());
  
  // 配置超时时间（针对快速测试调整）
  powerManager.setIdleTimeout(5000);      // 5 秒空闲超时
  powerManager.setSleepTimeout(15000);    // 15 秒睡眠超时
  
  Serial.println(\"\\nWaiting 10 seconds to demonstrate power state changes...\");
  for (int i = 0; i < 10; i++) {
    delay(1000);
    powerManager.update();
    Serial.printf(\"Idle Time: %lu ms, Power State: %d\\n\", 
                  powerManager.getIdleTime(), 
                  powerManager.getPowerState());
  }
}

/**
 * 示例 4: 监控连接状态和电池信息
 * 
 * 这个函数展示了如何监控设备状态
 */
void example_connection_monitoring() {
  Serial.println(\"\\n=== Connection Monitoring Example ===\\n\");
  
  // 模拟连接到 Phone A
  Serial.println(\"Connecting to Phone A...\");
  profileManager.switchProfile(0);
  profileManager.updateConnectionStatus(0, true);
  profileManager.updateBatteryLevel(85);
  profileManager.updateSignalStrength(-45);  // 强信号
  
  // 打印详细信息
  profileManager.printProfileInfo(0);
  
  // 稍后，模拟切换到 Phone B
  delay(2000);
  Serial.println(\"\\nDisconnecting from Phone A and connecting to Phone B...\");
  profileManager.updateConnectionStatus(0, false);
  
  profileManager.switchProfile(1);
  profileManager.updateConnectionStatus(1, true);
  profileManager.updateBatteryLevel(60);
  profileManager.updateSignalStrength(-70);  // 弱信号
  
  profileManager.printProfileInfo(1);
}

/**
 * 示例 5: 电源管理配置
 * 
 * 这个函数展示了如何自定义电源管理参数
 */
void example_power_configuration() {
  Serial.println(\"\\n=== Power Management Configuration ===\\n\");
  
  // 对于笔记本电脑使用（需要更多功率），使用更长的空闲超时
  Serial.println(\"Configuration for Laptop use:\");
  powerManager.setIdleTimeout(60000);      // 1 分钟空闲
  powerManager.setSleepTimeout(1800000);   // 30 分钟睡眠
  Serial.println(\"  - Idle timeout: 60 seconds\");
  Serial.println(\"  - Sleep timeout: 30 minutes\");
  
  delay(1000);
  
  // 对于手机使用（电池寿命是首要考虑），使用更短的超时
  Serial.println(\"\\nConfiguration for Mobile use:\");
  powerManager.setIdleTimeout(30000);      // 30 秒空闲
  powerManager.setSleepTimeout(300000);    // 5 分钟睡眠
  Serial.println(\"  - Idle timeout: 30 seconds\");
  Serial.println(\"  - Sleep timeout: 5 minutes\");
}

/**
 * 示例 6: 使用回调处理设备切换事件
 * 
 * 这个函数展示了如何使用回调函数响应设备切换事件
 */
void on_device_switched(uint8_t profileIndex) {
  Serial.printf(\"\\n[CALLBACK] Device switched to profile %d\\n\", profileIndex);
  
  DeviceProfile* profile = profileManager.getProfile(profileIndex);
  if (profile) {
    Serial.printf(\"Device Name: %s\\n\", profile->name);
    Serial.printf(\"Connected: %s\\n\", profile->isConnected ? \"YES\" : \"NO\");
    Serial.printf(\"Battery: %u%%\\n\", profile->batteryLevel);
  }
  
  // 你可以在这里添加额外的逻辑，例如：
  // - 发送特定的 BLE 命令
  // - 更新 UI 显示
  // - 触发其他设备操作
}

void example_callbacks() {
  Serial.println(\"\\n=== Device Switching Callbacks ===\\n\");
  
  // 设置回调函数
  deviceSwitcher.setSwitchCallback(on_device_switched);
  
  Serial.println(\"Setting up callback, now switching devices...\");
  deviceSwitcher.switchToProfile(0);
  delay(1000);
  
  deviceSwitcher.switchToProfile(1);
  delay(1000);
  
  deviceSwitcher.switchToProfile(2);
}

/**
 * 示例 7: 完整的初始化流程
 * 
 * 这个函数展示了正确的系统初始化顺序
 */
void example_full_initialization() {
  Serial.println(\"\\n=== Full System Initialization ===\\n\");
  
  // 步骤 1: 初始化所有管理器
  Serial.println(\"Step 1: Initializing managers...\");
  profileManager.initProfiles();
  powerManager.init();
  deviceSwitcher.init(SWITCH_MODE_BUTTON);
  
  // 步骤 2: 设置回调（可选）
  Serial.println(\"Step 2: Setting up callbacks...\");
  deviceSwitcher.setSwitchCallback(on_device_switched);
  
  // 步骤 3: 配置参数
  Serial.println(\"Step 3: Configuring parameters...\");
  powerManager.setIdleTimeout(45000);
  powerManager.setSleepTimeout(900000);
  
  // 步骤 4: 验证配置
  Serial.println(\"Step 4: Verifying configuration...\");
  profileManager.printAllProfiles();
  deviceSwitcher.printStatus();
  
  Serial.println(\"\\nSystem initialization complete!\\n\");
}

/**
 * 主程序中运行示例的方法：
 * 
 * void setup() {
 *   Serial.begin(115200);
 *   delay(500);
 *   
 *   // 根据需要运行示例
 *   // example_setup_profiles();
 *   // example_device_switching();
 *   // example_power_management();
 *   // example_connection_monitoring();
 *   // example_power_configuration();
 *   // example_callbacks();
 *   example_full_initialization();
 * }
 * 
 * void loop() {
 *   // 在实际应用中，主循环需要定期调用：
 *   powerManager.update();
 *   deviceSwitcher.update();
 *   delay(10);
 * }
 */
