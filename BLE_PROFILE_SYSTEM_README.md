# BBQ10 BLE 配置文件管理系统

这是一个高级的BLE（蓝牙）配置文件管理系统，为BBQ10键盘添加了多设备支持、快速切换、电源管理和连接状态感知。

## 🎯 核心功能

### 1. **多设备配置文件管理**
- 支持最多 **3 个设备配置文件**（手机A、手机B、笔记本）
- 每个配置文件存储：
  - 设备名称
  - BLE MAC 地址
  - 连接状态
  - 电池电量
  - 信号强度
  - 最后连接时间

### 2. **快速设备切换**
- **快捷键组合**：`ALT + SYM + 按键`
  - `ALT + SYM + W` → 切换到设备 0
  - `ALT + SYM + E` → 切换到设备 1
  - `ALT + SYM + R` → 切换到设备 2
- 500ms 防抖延迟，防止误触
- 支持3种切换模式（可配置）

### 3. **高级电源管理**
系统自动根据活动情况和连接状态调整功率：

| 状态 | CPU 频率 | 触发条件 |
|------|---------|--------|
| **ACTIVE** | 240 MHz | 有键盘输入 |
| **IDLE** | 160 MHz | 30秒无输入 |
| **CONNECTED** | 80 MHz | 2分钟无输入（已连接） |
| **SLEEP** | 40 MHz | 10分钟无输入 |
| **DEEP_SLEEP** | 1 MHz | 10分钟无输入（未连接） |

#### 电池优化特性：
- 📊 **实时电池监测** - ADC 读取电池电压
- 🔋 **自动功率缩放** - 根据活动状态动态调整 CPU 频率
- 😴 **智能睡眠** - 未连接时自动进入深度睡眠
- ⏰ **可配置超时** - 可调整各个时间阈值
- 🔔 **GPIO 唤醒** - 任何按键都能唤醒设备

### 4. **连接状态感知**
- 自动检测 BLE 连接/断开事件
- 更新配置文件的连接状态
- 触发相应的电源管理策略
- 自动重连到最后活跃的设备

## 📋 API 文档

### BLE Profile Manager (配置文件管理器)

#### 初始化
```cpp
profileManager.initProfiles();  // 从 EEPROM 加载配置
profileManager.loadProfilesFromEEPROM();
profileManager.saveProfilesToEEPROM();
```

#### 配置文件操作
```cpp
// 创建新配置文件
profileManager.createProfile(0, "Phone A", bleAddress);

// 切换活跃配置文件
profileManager.switchProfile(1);

// 删除配置文件
profileManager.deleteProfile(0);

// 获取配置文件
DeviceProfile* profile = profileManager.getProfile(0);
DeviceProfile* active = profileManager.getActiveProfile();
```

#### 状态查询
```cpp
// 检查配置文件有效性
bool valid = profileManager.isProfileValid(0);

// 获取活跃配置文件索引
uint8_t index = profileManager.getActiveProfileIndex();

// 获取有效配置文件数量
uint8_t count = profileManager.getProfileCount();

// 打印配置文件信息
profileManager.printProfileInfo(0);
profileManager.printAllProfiles();
```

#### 更新状态
```cpp
// 更新连接状态
profileManager.updateConnectionStatus(0, true);

// 更新信号强度（RSSI）
profileManager.updateSignalStrength(-50);  // dBm

// 更新电池电量
profileManager.updateBatteryLevel(75);  // 0-100%
```

### Power Manager (电源管理器)

#### 初始化与控制
```cpp
powerManager.init();           // 初始化电源管理
powerManager.setConnectionStatus(true);  // 设置连接状态
powerManager.recordActivity(); // 记录用户活动
powerManager.update();         // 更新电源状态
```

#### 睡眠控制
```cpp
// 手动控制睡眠
powerManager.enterSleep();
powerManager.exitSleep();
powerManager.enterDeepSleep();
```

#### 状态查询
```cpp
// 获取当前电源状态
PowerState state = powerManager.getPowerState();

// 检查是否睡眠
bool sleeping = powerManager.isAsleep();

// 获取空闲时间（毫秒）
uint32_t idleTime = powerManager.getIdleTime();

// 获取电池百分比
float battery = powerManager.getBatteryPercentage();
```

#### 配置
```cpp
// 设置空闲超时
powerManager.setIdleTimeout(45000);    // 45 秒

// 设置睡眠超时
powerManager.setSleepTimeout(900000);  // 15 分钟
```

### Device Switcher (设备切换器)

#### 初始化
```cpp
// 初始化切换器（SWITCH_MODE_BUTTON 为默认模式）
deviceSwitcher.init(SWITCH_MODE_BUTTON);
```

#### 切换操作
```cpp
// 切换到下一个设备
deviceSwitcher.switchToNext();

// 切换到上一个设备
deviceSwitcher.switchToPrevious();

// 切换到指定设备
deviceSwitcher.switchToProfile(1);
```

#### 状态查询
```cpp
// 获取当前活跃设备索引
uint8_t profile = deviceSwitcher.getCurrentProfile();

// 获取当前设备名称
const char* name = deviceSwitcher.getCurrentProfileName();

// 打印状态
deviceSwitcher.printStatus();
```

#### 回调
```cpp
// 设置切换事件回调
void onDeviceSwitch(uint8_t profileIndex) {
  Serial.printf("Switched to device: %d\n", profileIndex);
}

deviceSwitcher.setSwitchCallback(onDeviceSwitch);
```

## 💾 EEPROM 存储结构

```
地址范围         | 用途                    | 大小
0-3             | EEPROM 有效性标志       | 4 字节
4-7             | 当前活跃配置文件        | 4 字节
8-455           | 3 个设备配置文件        | 448 字节
                | (每个 149.3 字节)       |
```

### 配置文件结构（每个 64 字节）
```cpp
struct DeviceProfile {
  char name[32];              // 32 字节 - 设备名称
  uint8_t address[6];         // 6 字节  - BLE MAC 地址
  bool isActive;              // 1 字节  - 是否活跃
  bool isConnected;           // 1 字节  - 连接状态
  uint32_t lastConnectTime;   // 4 字节  - 最后连接时间
  uint32_t batteryLevel;      // 4 字节  - 电池电量 (0-100)
  uint8_t connectionStrength; // 1 字节  - 信号强度 (0-100)
};
```

## 🔧 配置与自定义

### 修改设备数量
编辑 `ble_profile_manager.h`：
```cpp
#define MAX_PROFILES 3  // 改为你需要的数量（最多推荐 5 个）
```

### 调整电源超时
编辑 `power_manager.h`：
```cpp
#define IDLE_TIMEOUT_MS 30000        // 空闲超时
#define CONNECTED_IDLE_TIMEOUT_MS 120000  // 连接时空闲超时
#define SLEEP_TIMEOUT_MS 600000      // 睡眠超时
```

### 更改设备切换快捷键
编辑 `main.cpp` 中的 `handleDeviceSwitching()` 函数：
```cpp
void handleDeviceSwitching() {
  // 修改按键位置和逻辑
  if (keyPressed(YOUR_COL, YOUR_ROW)) {
    deviceSwitcher.switchToProfile(0);
  }
}
```

### 选择切换模式
在 `setup()` 中：
```cpp
deviceSwitcher.init(SWITCH_MODE_CHORD);      // 组合按键模式
deviceSwitcher.init(SWITCH_MODE_SEQUENTIAL); // 顺序切换模式
deviceSwitcher.init(SWITCH_MODE_BUTTON);     // 单按钮模式
```

## 🚀 使用场景示例

### 场景 1: 在三个设备间快速切换
```
1. 设置三个配置文件：Phone A, Phone B, Laptop
2. 配对所有设备
3. 需要切换时：按 ALT + SYM + W/S/G
4. 系统自动断开当前连接，连接到新设备
```

### 场景 2: 延长电池寿命
```
- 键盘自动根据活动状态调整 CPU 频率
- 30秒无输入 → 降速到 160MHz
- 2分钟无输入（已连接）→ 降速到 80MHz  
- 10分钟无输入（已断开）→ 进入深度睡眠，等待唤醒
- 任何按键 → 立即从睡眠唤醒
```

### 场景 3: 电池监控
```cpp
// 在你的监控系统中：
float battery = powerManager.getBatteryPercentage();
if (battery < 10) {
  Serial.println("Low battery warning!");
}

// 查看连接的设备电池：
DeviceProfile* profile = profileManager.getActiveProfile();
printf("Current device battery: %d%%\n", profile->batteryLevel);
```

## 📊 调试与监控

### 打印所有配置信息
```cpp
profileManager.printAllProfiles();
```

### 打印设备切换器状态
```cpp
deviceSwitcher.printStatus();
```

### 查看单个配置文件
```cpp
profileManager.printProfileInfo(0);
```

### 获取详细状态信息
```cpp
Serial.printf("Battery: %.1f%%\n", powerManager.getBatteryPercentage());
Serial.printf("Power State: %d\n", powerManager.getPowerState());
Serial.printf("Idle Time: %lu ms\n", powerManager.getIdleTime());
```

## 🔌 硬件要求

- **ESP32-S3** 或兼容芯片
- **BLE 支持** - NimBLE-Arduino 库
- **EEPROM** - 至少 456 字节
- **电池监测** - ADC 输入（GPIO 34）
- **GPIO 唤醒** - GPIO 13（可配置）

## 🐛 故障排查

### 配置文件无法保存
- 检查 EEPROM 是否正确初始化
- 检查 `EEPROM.commit()` 是否被调用
- 使用 `profileManager.printAllProfiles()` 验证

### 设备切换不工作
- 验证两个设备都已配对
- 检查快捷键映射是否正确
- 查看串口输出中的切换日志

### 电池电量显示不准确
- 校准 ADC 读取（修改 `readBatteryVoltage()` 中的因子）
- 验证电池电压范围 (3V-4.2V)
- 检查 GPIO 34 连接是否正确

### 深度睡眠不工作
- 确保已调用 `powerManager.init()`
- 验证 GPIO 13 配置正确
- 检查 `esp_sleep_enable_gpio_wakeup()` 调用

## 📝 更新日志

### v1.0.0 (初始版本)
- ✅ 多设备配置文件管理
- ✅ 快速设备切换
- ✅ 电源状态管理（5 个等级）
- ✅ 连接状态感知
- ✅ EEPROM 持久化存储
- ✅ 电池监控
- ✅ 信号强度跟踪

## 📄 许可证

该代码为 BBQ10 项目的一部分，遵循相同的许可证。

---

**注意**：所有功能都在 ESP32-S3 BBQ10 键盘上测试。如果在其他硬件上使用，请根据需要调整 GPIO 引脚和参数。
