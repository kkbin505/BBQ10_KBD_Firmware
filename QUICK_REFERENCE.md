# BBQ10 BLE 配置文件系统 - 快速参考

## 🎮 快速切换指南

### 在设备间切换

**快捷键**: `ALT + SYM + 按键`

```
ALT + SYM + W    →  切换到设备 0 (Phone A)
ALT + SYM + E    →  切换到设备 1 (Phone B)  
ALT + SYM + R    →  切换到设备 2 (Laptop)
```

### 验证切换

查看 USB 连接的设备状态，或在串口监控器中查看：

```
[SWITCH] Switched to profile 0: Phone A
[BLE] Switched to profile 0: Phone A
```

---

## 🔋 电源状态说明

| 状态 | CPU 速度 | 触发条件 | 电池消耗 |
|------|---------|--------|--------|
| 🟢 **ACTIVE** | 240 MHz | 有按键输入 | ⚡⚡⚡ 最高 |
| 🟡 **IDLE** | 160 MHz | 30秒无输入 | ⚡⚡ 中等 |
| 🔵 **CONNECTED** | 80 MHz | 2分钟无输入（已连接） | ⚡ 较低 |
| 🟠 **SLEEP** | 40 MHz | 10分钟无输入 | ⚡ 低 |
| ⚫ **DEEP_SLEEP** | 1 MHz | 10分钟无输入（未连接） | 🔌 最低 |

---

## 📱 设备配置示例

### 创建配置

```cpp
// 创建新配置文件
profileManager.createProfile(0, "Phone A", bleAddress);
profileManager.createProfile(1, "Phone B", bleAddress);
profileManager.createProfile(2, "Laptop", bleAddress);
```

### 切换配置

```cpp
// 程序化切换
deviceSwitcher.switchToProfile(0);  // 切换到设备 0
```

---

## 🔌 硬件连接

### 电池监测

```
GPIO 34 (ADC1_CH6)  ──── 电池 (+)
GND                 ──── 电池 (-)
```

### 唤醒按钮

```
GPIO 13  ──── 任意键盘按键 (用于唤醒)
```

---

## 🐛 调试命令

### 打印所有配置

```cpp
profileManager.printAllProfiles();
```

输出示例：
```
=== BLE Device Profiles ===

--- Profile 0: Phone A ---
Active: YES
Connected: YES
Battery: 85%
Signal: 75%
Last Connect: 5 seconds ago
```

### 打印切换器状态

```cpp
deviceSwitcher.printStatus();
```

### 查询电池电量

```cpp
float battery = powerManager.getBatteryPercentage();
Serial.printf("Battery: %.1f%%\n", battery);
```

---

## 🔧 常见调整

### 修改空闲超时时间

```cpp
// 在 setup() 中
powerManager.setIdleTimeout(60000);  // 60 秒
```

### 修改睡眠超时时间

```cpp
// 在 setup() 中
powerManager.setSleepTimeout(1800000);  // 30 分钟
```

### 改变最大设备数

编辑 `ble_profile_manager.h`:
```cpp
#define MAX_PROFILES 5  // 改为 5 个设备
```

---

## 📊 连接状态流程

```
按键输入
    ↓
[POWER] 记录活动 → CPU 恢复 240 MHz
    ↓
[BLE] 检测连接状态
    ↓
├─ 已连接 → 更新配置信息
│          → 记录连接时间
│
└─ 未连接 → 尝试重连上次设备
           → 10分钟后进入深度睡眠
```

---

## ⏱️ 事件时间线示例

```
时刻      事件
0s       设备启动，连接到 Phone A (ACTIVE)
30s      无输入，降速到 160 MHz (IDLE)
2m       30s 后无输入，降速到 80 MHz (CONNECTED)
10m      还是无输入，进入深度睡眠 (DEEP_SLEEP)
        ↓ (等待任何键盘按键)
10m 5s   用户按键 → 唤醒，切换到 Laptop
        → 立即切换 → 连接建立
```

---

## 🔄 设备切换流程

```
按 ALT + SYM + W/E/R
    ↓
检查 ALT + SYM 状态 ✓
    ↓
检查防抖时间 (500ms) ✓
    ↓
检查目标配置有效性 ✓
    ↓
更新 profileManager.activeProfile
    ↓
保存到 EEPROM
    ↓
BLE 层处理重新连接
    ↓
输出日志: "[SWITCH] Switched to profile X: Device Name"
```

---

## 📝 EEPROM 结构

```
地址 (十六进制)   数据               字节数
0x00           EEPROM 魔数         4
0x04           活跃配置索引        4
0x08           配置文件 0          64
0x48           配置文件 1          64
0x88           配置文件 2          64
```

---

## 🚨 错误代码 / 日志

### BLE 模块

| 日志 | 含义 | 处理 |
|------|------|------|
| `[BLE] EEPROM initialized` | EEPROM 首次初始化 | 正常 |
| `[BLE] Loaded profiles` | 从 EEPROM 加载配置 | 正常 |
| `[BLE] Cannot switch to invalid profile` | 配置无效 | 创建配置 |
| `[BLE] Connected to profile X` | 连接成功 | 正常 |
| `[BLE] Disconnected` | 连接断开 | 等待重连 |

### 电源模块

| 日志 | 含义 | 处理 |
|------|------|------|
| `[POWER] State: ACTIVE` | CPU 全速 | 正常 |
| `[POWER] State: SLEEP` | 进入睡眠 | 电池保护 |
| `[POWER] Entering deep sleep` | 超时深度睡眠 | 等待唤醒 |
| `[POWER] Exiting sleep` | 被唤醒 | 继续工作 |

### 设备切换

| 日志 | 含义 | 处理 |
|------|------|------|
| `[SWITCH] Switched to profile X` | 切换成功 | 正常 |
| `[SWITCH] Cannot switch` | 防抖中 | 等待后重试 |
| `[SWITCH] Device: (name)` | 当前设备 | 显示状态 |

---

## 🎯 使用场景

### 场景 1: 工作环境（笔记本）
```
长时间不动，应该：
设置空闲超时: 5分钟
设置睡眠超时: 30分钟
CPU: 保持较高速度避免卡顿
```

### 场景 2: 移动工作（手机）
```
频繁切换，需要：
多个设备配置（3 个）
快速切换快捷键
较短的睡眠超时（5分钟）
```

### 场景 3: 低电池模式
```
启用所有电源优化：
立即进入 SLEEP 状态
最小化 BLE 扫描
必要时启用深度睡眠
```

---

## 📱 蓝牙连接清单

在使用此系统前，确保：

- [ ] 两个设备都已与 ESP32-S3 配对
- [ ] BLE 配置文件已正确创建
- [ ] BLE MAC 地址已保存
- [ ] EEPROM 已初始化
- [ ] 电池连接正确
- [ ] 所有库已安装

---

## 🔌 引脚参考

### 键盘矩阵

```
列: GPIO 1, 2, 3, 4, 5
行: GPIO 6, 7, 8, 9, 10, 11, 12
```

### 电源相关

```
电池 ADC: GPIO 34
唤醒按钮: GPIO 13 (可配置)
```

---

## 💾 备份与恢复

### 备份 EEPROM

```bash
esptool.py --port COM28 read_flash_status  # 检查状态
esptool.py --port COM28 dump_mem 0x8000 0x1000 backup.bin
```

### 恢复 EEPROM

```bash
esptool.py --port COM28 write_flash 0x8000 backup.bin
```

### 清空配置

```bash
esptool.py --port COM28 erase_flash
```

---

## 🆘 快速故障排查

| 问题 | 可能原因 | 解决方案 |
|------|--------|--------|
| 切换不工作 | 快捷键未激活 | 检查 ALT + SYM 按键 |
| 设备无法连接 | 配置未创建 | 运行 `profileManager.createProfile()` |
| 电池显示错误 | ADC 未正确校准 | 调整 `readBatteryVoltage()` 中的因子 |
| 频繁断开 | 电源管理激进 | 增加空闲超时时间 |
| 深度睡眠无法唤醒 | GPIO 13 配置错误 | 检查 `powerManager.init()` |

---

**最后更新**: 2026年4月
**快速帮助**: 按 `ALT + SYM + 问题` 获取帮助信息（待实现）
