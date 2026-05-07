# BBQ10KBD Firmware (ESP32-S3)

## ESP32-Powered Blackberry Q10 Keyboard with Web Configurator

I have always dreamed of building my own portable physical keyboard. After exploring the community, I was deeply inspired by the pioneering work of arturo182/BBQ10KBD and ZitaoTech/BBQ10-USB_BLE_Keyboard. Their research provided the foundation for this project, and I am incredibly grateful for their contributions to the open-source community.

![image](https://github.com/kkbin505/BBQ10_KBD_Firmware/blob/web_server/img/IMG_5940.jpg)

Project Overview
This project is a lightweight, ESP32-based firmware implementation designed for the Blackberry Q10 keyboard module. Unlike traditional firmware that requires recompiling to change keymaps, this project focuses on a streamlined user experience through an integrated WiFi-based configuration interface.

Key Features
ESP32-Based: Lightweight and efficient firmware architecture.

WiFi Web Configurator: No need to recompile the code. Simply connect to the keyboard’s WiFi to change settings on the fly.

<img width="647" height="401" alt="image" src="https://github.com/user-attachments/assets/54d68493-1265-4a96-b096-8f25551b9b86" />


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
4.  A configuration page should **automatically pop up** (Captive Portal). If not, visit `http://10.0.0.1` in your browser.
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
Ctrl(Mic) Win/Cmd(0) Space Sym(L2/L3) aA(Shift)
```

## Features & Optimizations

### 1. WiFi Web Dashboard
- **Visual Mapping**: A modern, interactive UI that mirrors your physical keyboard layout.
- **Multi-Layer View**: Toggle between Base, Symbol, Nav, and Long Press layers with dedicated tabs.
- **Dynamic Editor**: A streamlined, contextual editor that adapts to the active layer.
- **Config Export/Import**: Download your layout as a JSON file or upload others' configurations.
- **Long Press Shortcuts**: Define custom behaviors (e.g., long press `m` for `..`).
- **Captive Portal**: press sym for 5 seconds, connect to wifi, opens the config page upon WiFi connection (AP IP: `10.0.0.1`).

### 2. iOS Command (⌘) Modifier
- The **0** key is mapped to the **Command (GUI)** modifier by default.

### 3. Smart Tab Trigger
- **Long Press Alt**: Sends a **Tab** signal.

### 4. System Integration
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

```
## Reference:

https://github.com/arturo182/BBQ10KBD

https://github.com/ZitaoTech/BBQ10-USB_BLE_Keyboard

## Technical Details
- **Framework**: Arduino ESP32
- **Filesystem**: LittleFS
- **Networking**: AsyncWebServer with Captive Portal support
- **Config**: JSON serialization via ArduinoJson
