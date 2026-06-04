# bt2i2c — Bluetooth to I2C Keyboard Bridge

A Pico W that connects to BLE and Classic Bluetooth HID keyboards and
forwards keystrokes over I2C to a host, with an SPI display showing
connection status.

## How It Works

```
BLE Keyboard ──[Bluetooth LE/BT Classic]──>  Pico W (bt2i2c)
                                                │
                                      ┌─────────┼──────────┐
                                      ▼         ▼          ▼
                                  Display    I2C slave   UART debug
                                  (SPI)      (GP4/GP5)    (USB serial)
```

## Wiring

### Pico W

| Function   | GPIO | Pin | Connected to       |
|------------|------|-----|--------------------|
| I2C0 SDA   | GP4  | 6   | Host SDA           |
| I2C0 SCL   | GP5  | 7   | Host SCL           |
| GND        | GND  | 38  | Host GND           |
| USB        | —    | —   | Computer (debug)   |

### Display (ST7789, 240×240 round SPI)

| Display label | GPIO | Pin | Notes                |
|---------------|------|-----|----------------------|
| VCC           | VSYS | 39  | 5V (onboard reg)     |
| GND           | GND  | 38  |                      |
| SCL (SCK)     | GP2  | 4   | SPI0 clock           |
| SDA (MOSI)    | GP3  | 5   | SPI0 data            |
| RES           | GP14 | 19  | Reset                |
| DC            | GP16 | 21  | Data/Command select  |
| BL            | —    | —   | Backlight (optional) |

No CS pin needed — the display module ties CS to GND internally.

## Building & Flashing

```sh
cd build && cmake .. && make bt2i2c_ble
# Flash build/src/bt2i2c_ble.uf2 to Pico W
```

## UART Protocol

Each HID report from the keyboard is sent as a 9-byte frame:

```
Byte 0:     0xFE  (sync marker)
Byte 1:     Modifier bits (LCtrl=0x01, LShift=0x02, LAlt=0x04, LGui=0x08,
                           RCtrl=0x10, RShift=0x20, RAlt=0x40, RGui=0x80)
Byte 2:     Reserved (always 0)
Bytes 3-8:  Key codes (up to 6 simultaneous keys, 0 = empty)
```

115200 baud, 8N1. The receiver syncs on 0xFE — if a byte is lost, the
next 0xFE re-syncs (no checksum needed).

## Testing

1. Flash both Picos with their respective firmware
2. Wire sender GP4 → receiver GP5 + GND ↔ GND
3. Connect receiver USB to computer, open serial monitor (115200 baud)
4. Connect sender USB to computer, open second serial monitor for debug
5. The sender scans for BLE HID keyboards — press the keyboard's pair button
6. After pairing, keystrokes appear on the receiver's serial output:
   ```
   RAW: 00 00 04 00 00 00 00 00
   KEY press: A
   RAW: 00 00 00 00 00 00 00 00
   KEY release: A
   ```

## Notes

- **GP0/GP1 conflict**: Pico W's CYW43 uses GP0/GP1 for SPI flash — do not use
  these pins for anything.
- **Display VCC**: VSYS (~5V) works if the display module has an onboard
  regulator. For modules without one, use 3.3V (Pin 36) instead.
- **Power**: Powered via USB for debugging. Host connection is I2C (3.3V
  logic, 4.7kΩ pull-ups recommended on SDA/SCL).
