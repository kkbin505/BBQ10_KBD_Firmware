# BBQ10 BLE 配置文件系统 - 编译与部署指南

## 📦 文件结构

```
src/
├── main.cpp                      # 主程序（已更新）
├── ble_profile_manager.h         # 配置文件管理器头文件
├── ble_profile_manager.cpp       # 配置文件管理器实现
├── power_manager.h               # 电源管理器头文件
├── power_manager.cpp             # 电源管理器实现
├── device_switcher.h             # 设备切换器头文件
└── device_switcher.cpp           # 设备切换器实现

platformio.ini                     # 项目配置（已包含必要的库）
BLE_PROFILE_SYSTEM_README.md       # 完整文档
CONFIGURATION_EXAMPLES.md          # 配置示例
COMPILATION_GUIDE.md               # 本文件
```

## 🛠️ 编译步骤

### 前置要求

- **PlatformIO** 或 **Arduino IDE** 
- **ESP32-S3 Board Support**
- 必要的库（已在 `platformio.ini` 中定义）

### 方法 1: 使用 PlatformIO (推荐)

#### 1. 安装依赖库

打开终端并运行：

```bash
platformio lib install
```

或在 `platformio.ini` 中已经定义的库会自动安装。

#### 2. 编译项目

```bash
platformio run -e esp32s3-bbq10
```

#### 3. 上传到开发板

```bash
platformio run -e esp32s3-bbq10 --target upload
```

#### 4. 监控串口输出

```bash
platformio device monitor
```

### 方法 2: 使用 Arduino IDE

#### 1. 在 Arduino IDE 中打开项目

- 打开 `main.cpp` 文件

#### 2. 配置开发板

- 工具 → 开发板 → 选择 "ESP32-S3 Dev Module"
- 工具 → 端口 → 选择适当的 COM 端口

#### 3. 安装必要的库

在库管理器中搜索并安装：
- `BleKeyboard` (T-vK)
- `NimBLE-Arduino` (h2zero)

#### 4. 编译

- 点击验证/编译按钮

#### 5. 上传

- 点击上传按钮

## ⚙️ PlatformIO 配置说明

当前 `platformio.ini` 配置：

```ini
[env:esp32s3-bbq10]
platform = espressif32              # ESP32 平台
board = esp32-s3-devkitc-1          # 开发板型号
framework = arduino                  # Arduino 框架
lib_deps =
  T-vK/ESP32 BLE Keyboard @ ^0.3.2  # BLE 键盘库
  h2zero/NimBLE-Arduino @ ^1.4.2    # NimBLE 库
monitor_speed = 115200              # 串口波特率
upload_port = COM28                 # 上传端口（根据系统调整）
monitor_port = COM28                # 监控端口
monitor_dtr = 0                     # DTR 禁用
monitor_rts = 0                     # RTS 禁用
upload_speed = 460800               # 上传速率

board_upload.flash_size = 4MB       # 闪存大小
board_build.partitions = default.csv # 分区表

build_flags =                        # 编译标志
  -DBOARD_HAS_PSRAM                 # 启用 PSRAM 支持
  -DARDUINO_USB_MODE=1              # USB 模式
  -DARDUINO_USB_CDC_ON_BOOT=1       # USB CDC 启动
  -DUSE_NIMBLE                      # 使用 NimBLE
```

### 需要调整的参数

根据你的系统环境调整：

1. **upload_port / monitor_port**: 你的 ESP32 COM 端口
   - Windows: COM3, COM4, etc.
   - Linux: /dev/ttyUSB0, /dev/ttyACM0, etc.
   - macOS: /dev/cu.SLAB_USBtoUART, etc.

2. **upload_speed**: 根据你的 USB 线质量调整
   - 稳定性优先: 115200
   - 标准: 460800
   - 快速: 921600

## ✅ 验证编译

编译成功时会看到：

```
Compiling .pio/build/esp32s3-bbq10/src/main.cpp.o
Compiling .pio/build/esp32s3-bbq10/src/ble_profile_manager.cpp.o
Compiling .pio/build/esp32s3-bbq10/src/power_manager.cpp.o
Compiling .pio/build/esp32s3-bbq10/src/device_switcher.cpp.o
...
Linking .pio/build/esp32s3-bbq10/firmware.elf
Checking size .pio/build/esp32s3-bbq10/firmware.elf
...
Environment esp32s3-bbq10 → compiling [PASSED]
```

## 🔍 常见编译错误

### 错误 1: 库未找到
```
fatal error: BleKeyboard.h: No such file or directory
```
**解决方案:**
```bash
platformio lib install "T-vK/ESP32 BLE Keyboard"
```

### 错误 2: EEPROM 相关错误
```
undefined reference to 'EEPROM'
```
**解决方案:** EEPROM 库应该包含在 Arduino 框架中。检查 build flags 中是否包含必要的定义。

### 错误 3: 内存不足
```
undefined reference to 'malloc'
```
**解决方案:** 优化代码或增加堆大小。编辑 `platformio.ini`:
```ini
build_flags =
  -DBOARD_HAS_PSRAM
  -mfix-esp32-psram-cache-issue
```

### 错误 4: 端口错误
```
Serial port COM28 not found
```
**解决方案:** 更新 `platformio.ini` 中的正确端口，或使用：
```bash
platformio device list
```

## 🚀 上传后验证

### 1. 打开串口监控

```bash
platformio device monitor -e esp32s3-bbq10
```

### 2. 查看启动日志

应该看到类似的输出：

```
[SYSTEM] BBQ10 BLE Keyboard - Starting...
[EEPROM] EEPROM initialized with default profiles
[BLE] Profile manager initialized
[SYSTEM] Profile manager initialized
[POWER] Power manager initialized
[SWITCH] Device Switcher initialized in mode: 0
[SYSTEM] Device switcher initialized

=== BLE Device Profiles ===

--- Profile 0: ---
Active: NO
Connected: NO
Battery: 100%
Signal: 0%
Last Connect: 0 seconds ago
...
```

### 3. 测试设备切换

按 `ALT + SYM + W` 应该看到：

```
[SWITCH] Switched to profile 0: Phone A
```

## 📊 构建大小

典型的二进制文件大小：

| 组件 | 大小 |
|------|------|
| 主程序 | ~120 KB |
| 库 | ~800 KB |
| 总计 | ~920 KB (< 4 MB 闪存) |

## 🔐 高级配置

### 启用调试输出

添加到 `platformio.ini`:

```ini
build_flags =
  -DDEBUG_ENABLED
  -DDEBUG_LEVEL=5
```

### 自定义 EEPROM 大小

编辑 `ble_profile_manager.h`:

```cpp
#define EEPROM_TOTAL_SIZE (EEPROM_PROFILES_BASE + (MAX_PROFILES * EEPROM_PROFILE_SIZE))
```

然后在上传前运行擦除 EEPROM:

```bash
esptool.py --port COM28 erase_flash
```

### 性能优化

对于更快的编译，使用分布式编译：

```ini
[platformio]
build_cache_dir = .pio/build_cache
```

## 📝 编译检查清单

上传前确认：

- [ ] 所有库已安装 (`platformio lib install`)
- [ ] 正确的开发板型号 (`esp32-s3-devkitc-1`)
- [ ] 正确的 COM 端口已配置
- [ ] 源文件都在 `src/` 目录中：
  - [ ] main.cpp (已更新)
  - [ ] ble_profile_manager.h/cpp
  - [ ] power_manager.h/cpp
  - [ ] device_switcher.h/cpp
- [ ] 编译日志显示所有源文件都被编译
- [ ] 最终大小 < 4 MB (闪存大小)

## 🔧 故障排查

### 编译但上传失败

**症状:** 编译成功但上传超时

**解决方案:**
1. 检查 USB 连接
2. 降低 `upload_speed`
3. 尝试按住开发板上的 BOOT 按钮

### 上传后无串口输出

**症状:** 看不到任何启动日志

**解决方案:**
1. 检查 Serial Monitor 波特率 (115200)
2. 尝试重新连接 USB
3. 更新驱动程序
4. 使用不同的 USB 端口或 USB 线

### 内存泄漏或崩溃

**症状:** 设备在运行中重启

**解决方案:**
1. 检查 EEPROM 写入次数（可能损坏）
2. 增加堆栈大小
3. 检查是否存在无限循环

## 📚 参考资源

- [ESP32-S3 文档](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/)
- [PlatformIO 文档](https://docs.platformio.org/)
- [Arduino 框架](https://github.com/espressif/arduino-esp32)
- [BleKeyboard 库](https://github.com/T-vK/ESP32-BLE-Keyboard)
- [NimBLE 库](https://github.com/h2zero/NimBLE-Arduino)

## 💡 提示

1. **增量编译**: PlatformIO 只重新编译更改过的文件，加快编译速度
2. **CI/CD 集成**: 可以设置 GitHub Actions 自动编译检查
3. **版本控制**: 定期保存 `platformio.ini` 和源代码
4. **备份**: 在上传前备份工作 EEPROM 内容

---

**最后更新:** 2026年4月
**兼容版本:** PlatformIO 6.x+, Arduino IDE 1.8.x+
