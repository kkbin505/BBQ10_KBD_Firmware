# BBQ10KBD Firmware (ESP32-S3)

This firmware turns an ESP32-S3 board into a high-performance BLE/USB keyboard for the BlackBerry Q10 keyboard matrix. Optimized for iOS and iPadOS.

## Keyboard Layout

The firmware supports three logical layers:

- **Layer 1 (Base)**: Normal letters.
- **Layer 2 (Symbols)**: Triggered by a **single press** of the `SYM` key.
- **Layer 3 (Navigation)**: Triggered by a **double tap** of the `SYM` key. Includes arrow keys.

### Key Mapping Reference

#### Layer 1 (Base)
```text
Q W E R T Y U I O P
A S D F G H J K L Backspace
Shift Z X C V B N M ; Enter
Cmd(Alt) Space Sym(L2/L3) 
```

#### Layer 2 (Symbols)
```text
# 1 2 3 ( ) - - + @
* 4 5 6 / : ; ' " Backspace
Shift 7 8 9 ? ! , . $ Enter
Cmd(Alt) 0 Space Sym(L2/L3)
```

#### Layer 3 (Navigation)
```text
.  UP .  LEFT .  .  .  .  .  .
.  DOWN RIGHT .  .  .  .  .  .  .
.  .  .  .  .  .  .  .  .  .
```

## Features & Optimizations

### 1. iOS Command (⌘) Modifier
- The **Alt** key is mapped to the **Command (GUI)** modifier.
- Supports instant shortcuts: `Alt + C` (Copy), `Alt + V` (Paste), `Alt + Space` (Spotlight).

### 2. Smart Tab Trigger
- **Long Press Alt**: Sends a **Tab** signal.
- **Intelligent Conflict Resolution**:
    - If used as a modifier (e.g., `Alt + C`), the Tab trigger is cancelled.
    - If long-pressed alone, the Command signal is released *before* sending Tab to ensure a "clean" Tab output (preventing accidental App Switcher activation if not desired).

### 3. Hardware Workaround (Broken ROW7)
- Due to a hardware issue where ROW7 is disconnected, `F`, `J`, and `K` are inaccessible directly.
- **Workaround**: 
    - Long press `D` -> `F`
    - Long press `H` -> `J`
    - Long press `L` -> `K`
- This fallback works in both Layer 1 and Layer 2 (e.g., long press `5` -> `6`).

### 4. System Integration
- **BLE Device Name**: `BBQ10KBD`.
- **Dual Mode**: Sends key events to both BLE and USB HID simultaneously.
- **Auto-Reconnect**: Preserves BLE bonds for seamless pairing.

## Build and Upload

Requirement: [PlatformIO](https://platformio.org/)

```bash
# Build project
pio run

# Upload to ESP32-S3
pio run --target upload
```

## BLE Pairing

1. Upload firmware and reboot.
2. Search for BLE device: `BBQ10KBD`.
3. If connection issues occur, forget the device on your host and pair again.
