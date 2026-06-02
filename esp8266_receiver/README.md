# ESP8266 Receiver for BT2I2C Bridge

Polls key events from the Pico W BT2I2C bridge via I2C using the official
[BBQ10Keyboard](https://github.com/solderparty/arduino_bbq10kbd) Arduino library.

## Hardware Connections

| Pico W | ESP8266 | Description |
|--------|---------|-------------|
| GP4    | GPIO4 (D2) | I2C SDA |
| GP5    | GPIO5 (D1) | I2C SCL |
| GND    | GND        | Common ground |

**Note:** Only 3 wires! I2C at 100kHz, address `0x1F`.

## Installation

1. **Install ESP8266 Board Support** in Arduino IDE:
   - Add to Board Manager URLs: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
   - Search for "esp8266" and install

2. **Install the BBQ10Keyboard library:**
   - Arduino Library Manager: search "BBQ10Keyboard" by solderparty
   - Or clone: https://github.com/solderparty/arduino_bbq10kbd

3. **Open** `esp8266_receiver.ino` in Arduino IDE

4. **Select Board** (e.g. "NodeMCU 1.0 (ESP-12E Module)")

5. **Upload** and open Serial Monitor at 115200 baud

## Expected Output

```
=== ESP8266 BT2I2C Receiver ===

BT2I2C firmware v0.3 detected
Ready! Press keys on your BLE keyboard.

KEY PRESSED: A (0x04)
KEY RELEASED: A (0x04)
KEY PRESSED: B (0x05)
KEY RELEASED: B (0x05)
```
