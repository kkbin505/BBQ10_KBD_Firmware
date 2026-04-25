# BBQ10KBD Firmware (ESP32-S3)

This firmware turns an ESP32-S3 board into a BLE keyboard for the BlackBerry Q10 keyboard matrix.

## Keycap Layout Reference (from your design)

The key legend you shared can be represented as two logical layers:

- Layer 1: normal letters / base keys
- Layer 2: symbol layer (triggered by the `sym` key in firmware)

### Layer 1 (Base)

```text
Q W E R T Y U I O P
A S D F G H J K L Backspace
Shift Z X C V B N M ; Enter
Ctrl Alt Space Sym(Layer2) aA(Layer3)
```

### Layer 2 (Symbols on keycaps)

```text
# 1 2 3 ( ) - - + @
* 4 5 6 / : ; ' " Backspace
Shift 7 8 9 ? ! , . $ Enter
Ctrl 0 Space Sym(Layer2) aA(Layer3)
```

Notes:

- Current firmware uses toggle-to-activate Layer 2 behavior: press `sym` once to enter Layer 2, press again to exit.
- If a key has no Layer 2 mapping in firmware, it falls back to Layer 1 output.
- Ctrl is a hold modifier on `ROW7/COL2`.
- Both left and right Shift keys are enabled as hold modifiers.
- Because ROW7 is damaged on this hardware, `F/J/K` are currently emitted by long-press fallback on `D/H/L`.
- The same fallback also works on Layer 2: long press `5/:/"` emits `6/;/'`.

## Implemented Today

- BLE HID keyboard output using ESP32 BLE Keyboard.
- USB HID keyboard output enabled in parallel with BLE.
- NimBLE backend enabled for better ESP32-S3 compatibility.
- Matrix scan for 7 rows x 5 columns.
- Long-press fallback mapping for broken ROW7 keys:
  - long press `D` -> `F`
  - long press `H` -> `J`
  - long press `L` -> `K`
- Shift support as a true HID modifier (both left and right Shift keys work; host can detect Shift state for shortcuts/IME switching).
- Enter key mapping to HID Return.
- Backspace key mapping fixed to matrix `ROW4/COL5`.
- Symbol key support using press-to-toggle layer selection.
- Ctrl hold modifier support on `ROW7/COL2`.

## Connectivity Behavior

- Key events are sent to USB HID and BLE HID at the same time.
- BLE bonds are preserved across reboots, so the keyboard auto-reconnects to previously paired hosts.

## Hardware Notes

- If ROW7 is physically open (no continuity), `F/J/K` cannot be read directly.
- The long-press fallback is a software workaround for that hardware issue.

## Build and Upload

From this `firmware` folder:

```bash
pio run
pio run --target upload
```

## BLE Pairing

1. Upload firmware and reboot the board.
2. Pair with BLE device name: `BBQ10-BLE2`.
3. Open a text box and test typing.
4. If connected but typing does not work, remove old pairing and pair again.


## ToDo list

Megasafe

