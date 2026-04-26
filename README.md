# BBQ10KBD Firmware (ESP32-S3)

## ESP32-Powered Blackberry Q10 Keyboard with Web Configurator

I have always dreamed of building my own portable physical keyboard. After exploring the community, I was deeply inspired by the pioneering work of arturo182/BBQ10KBD and ZitaoTech/BBQ10-USB_BLE_Keyboard. Their research provided the foundation for this project, and I am incredibly grateful for their contributions to the open-source community.

![image](https://github.com/kkbin505/BBQ10_KBD_Firmware/blob/web_server/img/IMG_5940.jpg)

Project Overview
This project is a lightweight, ESP32-based firmware implementation designed for the Blackberry Q10 keyboard module. Unlike traditional firmware that requires recompiling to change keymaps, this project focuses on a streamlined user experience through an integrated WiFi-based configuration interface.

Key Features
ESP32-Based: Lightweight and efficient firmware architecture.

WiFi Web Configurator: No need to recompile the code. Simply connect to the keyboard’s WiFi to change settings on the fly.

![image](https://github.com/user-attachments/assets/ae8b1524-0572-4314-abdb-6f2c4adfec81)

![image](https://github.com/user-attachments/assets/fcecaf61-dc60-4fdc-9c34-d4b2fef50c74)

Graphical Interaction: An intuitive web-based interface for effortless key mapping and macro configuration.

Layer Customization: Fully supports custom layers, allowing you to define different key behaviors for various workflows or devices.

This project is a work-in-progress, and I hope it serves as a useful tool for others looking to build their own custom portable devices.

This firmware turns an ESP32-S3 board into a high-performance BLE/USB keyboard for the BlackBerry Q10 keyboard matrix. Optimized for iOS and iPadOS.

## 🚀 NEW: Dynamic Keyboard Configuration
No more re-compiling! You can now customize your keyboard layout and shortcuts instantly over WiFi.

### How to enter Config Mode
1.  **Hold the `SYM` key for 10 seconds.**
2.  The keyboard will temporarily disconnect Bluetooth and start a WiFi hotspot named **`BBQ10_Config`**.
3.  Connect to the WiFi (Password: `12345678`).
4.  A configuration page should **automatically pop up** (Captive Portal). If not, visit `http://192.168.4.1` in your browser.
5.  Customize your keys and click **Save**.
6.  Click **Reboot** to return to normal keyboard mode.

---

## Keyboard Layout

The firmware supports three logical layers by default, but all are fully customizable via the Web Dashboard:

- **Layer 1 (Base)**: Normal letters.
- **Layer 2 (Symbols)**: Triggered by a **single press** of the `SYM` key.
- **Layer 3 (Navigation)**: Triggered by a **double tap** of the `SYM` key. Includes arrow keys.

### Default Mapping Reference (Customizable)

#### Layer 1 (Base)
```text
Q W E R T Y U I O P
A S D F G H J K L Backspace
Shift Z X C V B N M ; Enter
Ctrl(Mic) Alt(0) Space Sym(L2/L3) aA(Shift)
```

## Features & Optimizations

### 1. WiFi Web Dashboard
- **Visual Mapping**: A modern, interactive UI that mirrors your physical keyboard layout.
- **Layer Editing**: Change characters for Base and Symbol layers directly.
- **Navigation Layer**: Map any key to special keycodes (Arrows, Home, End, etc.).
- **Long Press Shortcuts**: Define custom behaviors for long-pressing keys (useful for hardware workarounds or macros).
- **Captive Portal**: Automatically opens the config page upon WiFi connection.

### 2. iOS Command (⌘) Modifier
- The **Alt** key is mapped to the **Command (GUI)** modifier by default.
- Supports instant shortcuts: `Alt + C` (Copy), `Alt + V` (Paste), `Alt + Space` (Spotlight).

### 3. Smart Tab Trigger
- **Long Press Alt**: Sends a **Tab** signal.
- **Intelligent Conflict Resolution**: Cancel Tab if Alt is used as a modifier.

### 4. Hardware Fallbacks
- Optimized for boards with broken matrix lines (e.g., long press `D` for `F`). These are now configurable via the dashboard!

### 5. System Integration
- **Dual Mode**: Sends key events to both BLE and USB HID simultaneously.
- **Auto-Reconnect**: Preserves BLE bonds for seamless pairing.
- **Persistent Storage**: Settings are stored in **LittleFS** as a JSON file.
### LED Status Indicators
The device features an onboard WS2812 RGB LED (controlled via GPIO48) to provide real-time visual feedback on the device status:

🔴 Red: BLE Disconnected

🟢 Green: BLE Connected

🔵 Blue: WiFi Configuration Mode

## Build and Upload

Requirement: [PlatformIO](https://platformio.org/)

```bash
# Build project
pio run

# Upload firmware
pio run -t upload

# Upload filesystem data (Required for the Web Dashboard)
pio run -t uploadfs
```

## Technical Details
- **Framework**: Arduino ESP32
- **Filesystem**: LittleFS
- **Networking**: AsyncWebServer with Captive Portal support
- **Config**: JSON serialization via ArduinoJson
