# ESP8266 Receiver for BT2UART Bridge

This Arduino sketch allows an ESP8266 (like NodeMCU or Wemos D1 Mini) to receive HID keyboard frames from the Pico W BT2UART bridge and display them on the serial monitor.

## Hardware Connections

### Pico W to ESP8266 (NodeMCU pin names)

| Pico W Pin | ESP8266 Pin | Description |
|------------|-------------|-------------|
| GP4 (TX)   | RX          | UART RX (4th pin from USB: 3v3, GND, TX, RX) |
| GND        | GND         | Common ground |

**Note:** Only 2 wires required! The ESP8266 only receives data, no TX connection needed.

## Supported ESP8266 Boards

- **Generic ESP8266 boards** with RX pin (tested)
- **NodeMCU** (use D7 instead, modify sketch to set UART_RX_PIN to 13)
- **Wemos D1 Mini** (may need pin adjustment)

## Installation

1. **Install Arduino IDE** (if not already installed)
2. **Install ESP8266 Board Support:**
   - Open Arduino IDE
   - Go to `File → Preferences`
   - Add to "Additional Board Manager URLs":
     ```
     http://arduino.esp8266.com/stable/package_esp8266com_index.json
     ```
   - Go to `Tools → Board → Boards Manager`
   - Search for "esp8266" and install "ESP8266 Boards" by ESP8266 Community

3. **Open the Sketch:**
   - Open `esp8266_receiver.ino` in Arduino IDE

4. **Select Board:**
   - Go to `Tools → Board → ESP8266 Boards`
   - Select "NodeMCU 1.0 (ESP-12E Module)" or your specific board

5. **Configure Serial Port:**
   - Go to `Tools → Port` and select your ESP8266's COM port

## Upload and Test

1. **Connect ESP8266 to Computer** via USB
2. **Upload the Sketch:**
   - Click the Upload button (→) in Arduino IDE
3. **Open Serial Monitor:**
   - Click Serial Monitor icon (magnifying glass)
   - Set baud rate to **115200**
4. **Power Pico W** with BT2UART firmware
5. **Connect Pico W GP4 to ESP8266 GPIO13 (D7)**
6. **Connect GND between Pico W and ESP8266**
7. **Pair your BLE keyboard** with the Pico W
8. **Start typing** on the BLE keyboard!

## Expected Output

```
=== ESP8266 BT2UART Receiver ===
Waiting for HID frames from Pico W...
Press keys on your BLE keyboard!

UART1 initialized on GPIO13 (RX)
Ready!

RAW: 00 00 04 00 00 00 00 00
KEY press: A (0x04) -> 'a'

RAW: 00 00 00 00 00 00 00 00
KEY release: A (0x04)

RAW: 02 00 05 00 00 00 00 00
Modifiers: [LShift]
KEY press: B (0x05) -> 'B'
```

## Wiring Diagram

```
Pico W (BT2UART)          ESP8266
┌─────────────┐          ┌─────────────┐
│             │          │  (USB port) │
│  GP4 (TX) ──┼──────────┼─── RX       │
│             │          │             │
│  GND ───────┼──────────┼─── GND      │
│             │          │             │
│  (USB power)│          │  (USB power)│
└─────────────┘          └─────────────┘
```

## Troubleshooting

### No output in Serial Monitor
- **Check baud rate:** Must be 115200
- **Check RX pin:** Use the dedicated RX pin (4th from USB: 3v3, GND, TX, RX)
- **Check wiring:** GP4 → RX, GND → GND
- **Test Pico W:** Check that Pico W is sending data (use another Pico as receiver)

### Garbage characters in output
- **Baud rate mismatch:** Both devices must use 115200
- **Ground connection missing:** Ensure GND is connected between devices

### ESP8266 won't upload sketch
- **Hold FLASH button:** Press and hold FLASH button while clicking Upload
- **Check driver:** Install CH340 driver if needed (NodeMCU)
- **Try different USB cable:** Some cables are power-only

### Missing key presses
- **Buffer overflow:** The simple parser may miss rapid key presses
- **UART errors:** Check wire length (keep short, <10cm)

## Features

- ✅ **HID report parsing** - Full 8-byte HID report decoding
- ✅ **Key name display** - Shows human-readable key names
- ✅ **ASCII conversion** - Converts to printable characters when possible
- ✅ **Modifier key support** - Shows Shift, Ctrl, Alt, GUI combinations
- ✅ **Frame synchronization** - Auto-resyncs using 0xFE marker
- ✅ **Duplicate filtering** - Only shows new key events
- ✅ **Debug output** - Raw hex dump + interpreted key events

## Limitations

- **Simple parser:** Doesn't handle all special keys (F-keys, multimedia, etc.)
- **No debouncing:** May show multiple events for held keys
- **Buffer-based:** May miss rapid key presses (typo rate)
- **Display only:** Doesn't forward keystrokes anywhere

## Customization

### Change RX Pin (for non-NodeMCU boards)
```cpp
#define UART_RX_PIN 3  // Change to your RX pin
```

### Enable verbose debugging
```cpp
#define DEBUG_MODE  // Uncomment at top of file
```

### Adjust baud rate (if you change Pico W firmware)
```cpp
#define BAUD_RATE 9600  // Match Pico W UART speed
```

## License

Same license as the main BT2UART project.