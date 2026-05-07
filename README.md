# BBQ10KBD Firmware

> **Transform your BlackBerry Q10 keyboard into a modern wireless keyboard with WiFi configuration—no soldering required, no recompiling needed.**

![Status](https://img.shields.io/badge/status-beta-yellow?style=flat-square)
![License](https://img.shields.io/badge/license-MIT-green?style=flat-square)
![Platform](https://img.shields.io/badge/platform-ESP32--S3-blue?style=flat-square)
![Hardware](https://img.shields.io/badge/hardware-BBQ10%20Keyboard-lightgrey?style=flat-square)

![BBQ10 Keyboard Project](https://github.com/kkbin505/BBQ10_KBD_Firmware/blob/web_server/img/IMG_5940.jpg)

## About This Project

This is a lightweight, ESP32-S3-based firmware implementation that transforms a BlackBerry Q10 keyboard module into a **high-performance dual-mode BLE/USB keyboard**. Unlike traditional keyboard firmwares that require recompiling to change keymaps, this project features an **integrated WiFi-based configuration interface**—simply hold a key to enter config mode, connect to the built-in WiFi hotspot, and customize your layout through an intuitive web dashboard.

**Built for portability and ease of use.** Optimized for iOS and iPadOS with intelligent modifier handling and designed by makers, for makers.

### ✨ Key Highlights

- 🔌 **Dual Connectivity**: Simultaneous BLE + USB HID output (works with phones, tablets, and computers)
- 📡 **WiFi Configuration**: No recompilation needed—customize keymaps instantly via web interface
- 🎨 **Visual Keyboard Editor**: Drag-and-drop key mapping with live preview
- 🎛️ **Multi-Layer System**: 3 customizable layers (Base, Symbols, Navigation) + long-press macros
- 📱 **iOS Optimized**: Pre-configured Command (⌘) modifier mapping
- 💾 **Persistent Storage**: Settings saved as JSON in LittleFS (survives power cycles)
- 🚀 **Status LEDs**: Real-time RGB feedback (WiFi, Bluetooth, battery status)
- 🔋 **Efficient**: Battery-friendly design with auto-sleep after 10 minutes inactivity

---

## 📚 Table of Contents

- [Hardware Requirements & Bill of Materials](#hardware-requirements--bill-of-materials)
- [Quick Start](#quick-start)
- [Configuration Guide](#configuration-guide)
- [Keyboard Layers](#keyboard-layers)
- [Features & Optimizations](#features--optimizations)
- [Architecture & Components](#architecture--components)
- [Building & Flashing](#building--flashing)
- [Troubleshooting & FAQ](#troubleshooting--faq)
- [Contributing](#contributing)
- [License & Credits](#license--credits)

---

## Hardware Requirements & Bill of Materials

### Core Components

| Component | Part Number / Details | Estimated Cost | Notes |
|-----------|----------------------|-----------------|-------|
| **Microcontroller** | ESP32-S3 DevKit-C-1 | $8–12 | 4MB Flash, WiFi + BLE, USB native |
| **Keyboard Module** | BlackBerry Q10 Keyboard | $15–25 | 7×5 matrix, find on eBay/AliExpress |
| **RGB Status LED** | WS2812B (NeoPixel) | $0.50–1 | Addressable RGB, GPIO48 on ESP32-S3 |
| **USB Cable** | USB-C (for ESP32-S3) | $1–2 | For flashing and power |
| **Micro USB Adapter** | (Optional) | $1–2 | If using USB-A keyboard connection |
| **Resistors** | 10k Ω pull-ups | $0.10 | For keyboard matrix lines (if needed) |

### Estimated Total: $25–45 USD

### Optional Components

- **Power Bank**: 5000mAh USB-C for portable use
- **3D-Printed Case**: Design provided (check `/data/` folder)
- **Solder & Header Pins**: For custom wiring

---

## Quick Start

Get your keyboard up and running in under 10 minutes.

### Prerequisites

✅ ESP32-S3 board with USB-C port  
✅ BlackBerry Q10 keyboard module  
✅ Computer with PlatformIO installed ([Get PlatformIO](https://platformio.org/))  
✅ USB-C cable for flashing  

### 1. Flash the Firmware

```bash
# Clone the repository
git clone https://github.com/kkbin505/BBQ10_KBD_Firmware.git
cd BBQ10_KBD_Firmware

# Build and upload with PlatformIO
pio run -e esp32s3-bbq10 -t upload

# Monitor serial output (optional, for debugging)
pio device monitor -b 115200
```

### 2. Pair Bluetooth

1. On your device (iOS, Android, Mac, Windows), open Bluetooth settings
2. Wait 2-5 seconds after power-on for the keyboard to broadcast
3. Select **`BBQ10KBD`** from the Bluetooth device list
4. Confirm pairing if prompted

Expected LED feedback: 🟢 **Green** = Connected

### 3. Enter Configuration Mode

1. **Hold the `SYM` key for 10 seconds**
2. LED turns 🔵 **Blue** (WiFi mode activated)
3. Your device''s WiFi settings will show a new network: **`BBQ10_Config`**
4. Connect to `BBQ10_Config` (Password: `12345678`)
5. A configuration page will **automatically pop up** (Captive Portal)
   - If not: manually visit `http://10.0.0.1` in your browser
6. Customize your key layout, then click **Save**
7. Click **Reboot** to return to normal keyboard mode

Expected LED feedback: LED returns to 🟢 **Green** after reboot

**That''s it!** Your keyboard is now ready to use with your custom layout.

---

## Configuration Guide

### Entering Configuration Mode

The configuration interface is accessed via WiFi hotspot:

1. Hold `SYM` key for **10 seconds** → LED turns 🔵 Blue
2. Connect to WiFi: **`BBQ10_Config`** (Password: **`12345678`**)
3. Open browser to: **`http://10.0.0.1`** (Captive Portal will auto-launch)
4. Customize your settings
5. Click **Save** then **Reboot** to apply changes

### Web Dashboard Features

- **Visual Keyboard Layout**: Click any key to remap it
- **Multi-Layer Tabs**: Switch between Base, Symbols, Navigation, and Long-Press layers
- **Key Type Selector**: Choose from standard keys, modifiers, macros, or special actions
- **Long-Press Macros**: Define behaviors for keys held >500ms (e.g., long-press `M` → `..`)
- **Import/Export**: Download your config as JSON or upload others'' configurations
- **Real-Time Preview**: See changes before saving

### LED Status Indicators

| Color | Meaning | Action |
|-------|---------|--------|
| 🔴 Red | BLE Disconnected | Pair via Bluetooth settings |
| 🟢 Green | BLE Connected | Ready to use |
| 🔵 Blue | WiFi Configuration Mode | Connect to `BBQ10_Config` to configure |

---

## Keyboard Layers

The firmware supports three logical layers, all fully customizable:

### Layer 1: Base Layer

Default alphanumeric layout:

```text
Q W E R T Y U I O P
A S D F G H J K L Backspace
Shift Z X C V B N M ; Enter
Ctrl(Mic) Win/Cmd(0) Space Sym(L2/L3) Shift
```

### Layer 2: Symbol Layer

Triggered by **single press** of `SYM` key. Default includes punctuation and symbols.

### Layer 3: Navigation Layer

Triggered by **double tap** of `SYM` key. Default includes arrow keys and navigation.

**All layers are fully customizable via the web dashboard.** No coding required.

---

## Features & Optimizations

### 1. WiFi Web Dashboard
- **Visual Mapping**: Interactive UI mirrors your physical keyboard layout
- **Multi-Layer View**: Toggle between Base, Symbol, Nav, and Long Press layers
- **Drag-and-Drop Editor**: Click any key to reassign it
- **Config Export/Import**: Download layouts as JSON for backup/sharing
- **Long Press Customization**: Define behaviors for held keys
- **Captive Portal**: Auto-opens on WiFi connection (also at `10.0.0.1`)

### 2. iOS Command (⌘) Modifier
- **Alt key** pre-mapped to **Command (GUI)** modifier on iOS/macOS
- Instant shortcuts: `Cmd + C` (Copy), `Cmd + V` (Paste), `Cmd + Space` (Spotlight)
- Fully customizable via web dashboard

### 3. Smart Tab Trigger
- **Long Press Alt**: Sends Tab signal for faster navigation
- **Intelligent Conflict Resolution**: Cancels Tab if Alt is used as a modifier key

### 4. Hardware Fallbacks
- Supports boards with broken matrix lines (e.g., remap broken `F` key to `D` long-press)
- All fallback mappings configurable via dashboard

### 5. Dual-Mode Connectivity
- **Simultaneous BLE + USB HID**: Send keys to both Bluetooth and USB devices at once
- **Auto-Reconnect**: Preserves BLE bonds for seamless re-pairing
- **Persistent Storage**: Settings stored as JSON in LittleFS (survives reboots)

### 6. Efficient Power Management
- **10-Minute Auto-Sleep**: Keyboard enters sleep mode after inactivity
- **Wake on Key Press**: Press any key to wake

---

## Architecture & Components

### System Overview

The firmware is organized into modular components:

```
ESP32-S3 Firmware
├── ConfigManager     → Manages keymaps (JSON in LittleFS)
├── StatusLED         → RGB LED feedback (GPIO48)
├── WebManager        → WiFi hotspot + captive portal
├── BLE Stack         → NimBLE for wireless pairing
├── USB HID Stack     → Native USB keyboard output
└── Main Loop         → Keyboard matrix scanning + HID dispatch
```

### Key Components

| Component | Responsibility | File |
|-----------|-----------------|------|
| **ConfigManager** | Store/load keyboard config (3 layers + macros) as JSON via LittleFS | `include/ConfigManager.h` |
| **WebManager** | WiFi AP mode, captive portal, web server for configuration UI | `include/WebManager.h` |
| **StatusLED** | RGB LED control (Red=disconnected, Green=connected, Blue=config mode) | `include/StatusLED.h` |
| **Main Loop** | Matrix scanning (7 rows × 5 cols), key debouncing, HID output dispatch | `src/main.cpp` |
| **BLE Handler** | Bluetooth HID pairing & disconnection logic | embedded in `main.cpp` |
| **USB Handler** | USB HID keyboard output (simultaneous with BLE) | embedded in `main.cpp` |

### Technology Stack

| Layer | Technology | Details |
|-------|-----------|---------|
| **Framework** | Arduino ESP32 | Official Arduino core for ESP32 |
| **BLE** | NimBLE-Arduino v1.4.2 | Efficient Bluetooth LE stack |
| **WiFi** | ESP32 Native WiFi | Built-in WiFi for hotspot + web server |
| **Web Server** | ESPAsyncWebServer v3.1.5 | Non-blocking async HTTP server |
| **Config Storage** | ArduinoJson v7.0.4 + LittleFS | JSON serialization in flash |
| **LED Control** | Adafruit NeoPixel v1.12.0 | WS2812B RGB LED driver |
| **Build System** | PlatformIO | Embedded development framework |

### Project Structure

```
BBQ10_KBD_Firmware/
├── platformio.ini           # Build config + dependencies
├── src/
│   ├── main.cpp            # Main firmware loop, matrix scanning
│   ├── ConfigManager.cpp   # Config persistence layer
│   ├── WebManager.cpp      # WiFi + web server
│   └── StatusLED.cpp       # LED control logic
├── include/                # Header files
│   ├── ConfigManager.h
│   ├── WebManager.h
│   └── StatusLED.h
├── data/                   # Web UI files
│   └── index.html         # Configuration dashboard (served via WebManager)
└── README.md              # This file
```

---

## Building & Flashing

### Requirements

- **PlatformIO Core** or **PlatformIO IDE** ([Install here](https://platformio.org/platformio-ide))
- **ESP32-S3 board** with 4MB flash
- **USB-C cable** for flashing

### Build Steps

```bash
# Clone repository
git clone https://github.com/kkbin505/BBQ10_KBD_Firmware.git
cd BBQ10_KBD_Firmware

# Build project
pio run -e esp32s3-bbq10

# Upload to ESP32-S3
pio run -e esp32s3-bbq10 -t upload

# Monitor serial output (115200 baud)
pio device monitor -b 115200 --filter colorlog

# Clean build (if issues)
pio run -e esp32s3-bbq10 -t clean
```

### Configuration (if needed)

Edit `platformio.ini` to adjust:

- **`upload_port`**: Serial port for flashing (auto-detected on most systems)
- **`monitor_port`**: Serial port for debugging
- **Build flags**: Enable/disable features via `-D` flags

---

## Troubleshooting & FAQ

### LED is Red (BLE Disconnected)

**Problem**: LED shows 🔴 Red constantly  
**Solution**:
1. Check Bluetooth is enabled on your device
2. Force-unpair the keyboard from Bluetooth settings
3. Restart the keyboard (power cycle)
4. Re-pair: Open Bluetooth settings and select `BBQ10KBD`
5. If still fails, check serial monitor output for errors: `pio device monitor`

### WiFi Config Hotspot Won''t Appear

**Problem**: Can''t see `BBQ10_Config` WiFi network  
**Solution**:
1. Confirm you''re holding `SYM` key for **10 full seconds** (count slowly)
2. Check LED turns 🔵 Blue to confirm WiFi mode activated
3. Restart keyboard (power cycle)
4. If ESP32-S3 board doesn''t have WiFi: Ensure `DBOARD_HAS_PSRAM` flag is enabled in `platformio.ini`
5. Check serial monitor for WiFi startup errors

### Can''t Connect to `http://10.0.0.1`

**Problem**: Captive portal doesn''t appear; manual URL times out  
**Solution**:
1. Confirm you''re connected to `BBQ10_Config` WiFi network
2. Try opening `http://10.0.0.1:80` explicitly
3. Disable WiFi proxy/VPN on your device temporarily
4. Clear browser cache: Try in private/incognito mode
5. If on Android: Check if "Captive Portal Bypass" is disabled in network settings
6. Restart the device

### Configuration Changes Don''t Save

**Problem**: Settings revert after reboot  
**Solution**:
1. Ensure you clicked **Save** in the web dashboard (green checkmark should appear)
2. Check that LED turns 🟢 Green after clicking **Reboot**
3. If using custom layers: Confirm you saved each layer individually before rebooting
4. Check device has free storage: Open serial monitor to see LittleFS usage
5. If corruption suspected: Factory reset via USB (erase LittleFS)

### Keyboard Keys Not Registering

**Problem**: Some or all keys don''t work after flashing  
**Solution**:
1. Check LED is 🟢 Green (Bluetooth connected)
2. Try a different Bluetooth device to rule out pairing issues
3. Test with USB connection simultaneously (keyboard should send to both)
4. Verify hardware connections: Keyboard matrix wires properly soldered
5. Check serial monitor: `pio device monitor` for matrix scan errors
6. Re-flash firmware: `pio run -e esp32s3-bbq10 -t upload`

### Keyboard Randomly Disconnects

**Problem**: BLE connection drops unexpectedly  
**Solution**:
1. Update to latest ESP32 BLE Keyboard library: `pio lib update`
2. Move keyboard closer to paired device (range issue)
3. Check for WiFi interference: Switch WiFi hotspot off when not configuring
4. Reduce BLE transmission power if device has radio noise nearby
5. Factory reset Bluetooth: Remove device from Bluetooth settings, re-pair

### Web Dashboard is Unresponsive or Slow

**Problem**: Configuration page hangs or takes long to load  
**Solution**:
1. Restart keyboard (exit WiFi mode by pressing any key + Reboot button)
2. Refresh browser (`Ctrl+R` or `Cmd+R`)
3. Try a different browser or device
4. Reduce simultaneous connections: Close other tabs/devices
5. Check WiFi signal strength at keyboard''s location

### Build Fails with Compiler Errors

**Problem**: `pio run` returns compilation errors  
**Solution**:
1. Clean build: `pio run -e esp32s3-bbq10 -t clean`
2. Update PlatformIO: `pio upgrade`
3. Reinstall dependencies: `pio lib update`
4. Check ESP32 board support is installed: `pio boards | grep esp32s3`
5. Verify `platformio.ini` has correct board: `board = esp32-s3-devkitc-1`

### Upload Fails / Board Not Found

**Problem**: `pio run -t upload` can''t find serial port  
**Solution**:
1. Connect ESP32-S3 via USB-C cable
2. List available ports: `pio device list`
3. Manually set port in `platformio.ini`: `upload_port = COM3` (Windows) or `/dev/ttyUSB0` (Linux)
4. Try different USB cable (data cable, not power-only)
5. Install CH340/CP210x drivers if using generic ESP32 boards
6. Hold "Boot" button while flashing if board doesn''t auto-reset

### My Keyboard Layout Isn''t Loading on Startup

**Problem**: Config saved but keyboard reverts to default layout  
**Solution**:
1. Check serial monitor for LittleFS mount errors
2. Verify JSON config file isn''t corrupted: Restart and check LED sequence
3. Try exporting config JSON from web dashboard, inspect for syntax errors
4. Re-upload config from web dashboard (don''t just click Save)
5. If persistent: Erase LittleFS and reconfigure: `pio run -t erase`

---

## Contributing

Contributions are welcome! Whether you''re fixing bugs, adding features, or improving documentation, here''s how to help.

### Development Setup

1. **Clone and set up locally**:
   ```bash
   git clone https://github.com/kkbin505/BBQ10_KBD_Firmware.git
   cd BBQ10_KBD_Firmware
   pip install platformio
   pio run -e esp32s3-bbq10  # Verify build
   ```

2. **Create a feature branch**:
   ```bash
   git checkout -b feature/your-feature-name
   ```

3. **Make your changes** and test thoroughly:
   ```bash
   pio run -e esp32s3-bbq10 -t upload
   pio device monitor  # Test and debug
   ```

### Contribution Guidelines

- **Code Style**: Follow Arduino conventions; use clear variable names
- **Commits**: Write descriptive commit messages (what + why, not just what)
- **Documentation**: Update README if adding features
- **Testing**: Test on actual ESP32-S3 hardware; describe your setup in PR
- **No Breaking Changes**: Ensure configs remain backward-compatible

### Areas for Contribution

- 🐛 **Bug Reports**: File issues with reproduction steps
- ✨ **Feature Ideas**: Suggest improvements in GitHub Discussions
- 📖 **Documentation**: Improve guides or add troubleshooting tips
- 🔧 **Code Improvements**: Optimize drivers, refactor components
- 🎨 **UI/UX**: Enhance the web dashboard (HTML/CSS/JavaScript in `/data/`)

---

## License & Credits

### License

This project is licensed under the **MIT License**. See [LICENSE](LICENSE) file for details.

You are free to use, modify, and distribute this firmware for personal, educational, or commercial purposes.

### Acknowledgments & Credits

This project stands on the shoulders of giants. Special thanks to:

- **[arturo182/BBQ10KBD](https://github.com/arturo182/BBQ10KBD)** - Original BBQ10 keyboard firmware research & implementation
- **[ZitaoTech/BBQ10-USB_BLE_Keyboard](https://github.com/ZitaoTech/BBQ10-USB_BLE_Keyboard)** - BLE/USB dual-mode keyboard architecture insights

The open-source community''s work on ESP32 BLE and USB HID has been invaluable. This project is an extension of their vision: making custom keyboards accessible to everyone.

---

## 📞 Support & Feedback

- 💬 **Issues**: [GitHub Issues](https://github.com/kkbin505/BBQ10_KBD_Firmware/issues)
- 💡 **Ideas**: [GitHub Discussions](https://github.com/kkbin505/BBQ10_KBD_Firmware/discussions)
- 🐦 **Follow Updates**: Star this repo for notifications

---

**Happy typing!** 🎹
