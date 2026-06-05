# BT2I2C Bridge

BLE HID keyboard → Pico W → I2C → Host.

Connects to a Bluetooth keyboard and forwards keystrokes as an I2C
slave using the standard [i2c_puppet](https://github.com/grymoire/i2c_puppet-Linux)
register protocol.

This firmware emulates a BBQ20-compatible I2C keyboard endpoint at address `0x1F`.
Use existing BBQ20 host support (for example Arduino/CircuitPython BBQ10KBD
libraries and other i2c_puppet-compatible clients) on the host side.

## Quick Start

```sh
cd build && cmake .. && make bt2i2c
```

Flash `build/src/bt2i2c.uf2` to a Pico W. Connect GP4 (SDA) and GP5 (SCL)
to your host's I2C bus with a common GND. The device appears at I2C address
`0x1F`. Use an existing BBQ10KBD library to poll `REG_ID_KEY` (0x04) for
FIFO count and `REG_ID_FIF` (0x09) for key events (2 bytes: state + keycode).

## Project Structure

```
src/bt_keyboard.c    BLE scan, pair, HID report → I2C FIFO
src/main.c           Init I2C slave + BT stack + run loop
src/fifo.c           Key event FIFO (31 entries)
src/reg.c            i2c_puppet register map
src/i2c_slave.c      I2C slave on i2c0 (GP4 SDA, GP5 SCL)
src/pins.h           Pin assignments
```

## Protocol

The device implements a subset of the i2c_puppet register map:

| Register | Addr | Size | Description |
|----------|------|------|-------------|
| `REG_ID_VER` | 0x01 | 1B | Firmware version |
| `REG_ID_CFG` | 0x02 | 1B | Configuration bits |
| `REG_ID_INT` | 0x03 | 1B | Interrupt status |
| `REG_ID_KEY` | 0x04 | 1B | FIFO count (lower 5 bits) |
| `REG_ID_FIF` | 0x09 | 2B | Key event: state + keycode |
| `REG_ID_ADR` | 0x12 | 1B | I2C address (read/write, default 0x1F) |

Key state values: `1=pressed`, `2=held`, `3=released`. Keycodes are
standard HID usage IDs. Write to a register by OR-ing `0x80` with its
address.

I2C at 100kHz, address `0x1F`. Internal pull-ups enabled (external 4.7kΩ
recommended for longer wires).

### Extended Keycodes (Protocol Extension)

Standard BBQ20 firmware uses ASCII codes (0x20-0x7E) for printable keys and
control codes (0x01-0x1D) for modifier/joystick keys. This firmware adds
keys that a full keyboard has (Escape, cursor arrows, F-keys, etc.) using
a **protocol extension**: codes in the 0x80+ range.

| Code | Key |
|------|-----|
| 0x80 | Escape |
| 0x81 | F1 |
| 0x82 | F2 |
| 0x83 | F3 |
| 0x84 | F4 |
| 0x85 | F5 |
| 0x86 | F6 |
| 0x87 | F7 |
| 0x88 | F8 |
| 0x89 | F9 |
| 0x8A | F10 |
| 0x8B | F11 |
| 0x8C | F12 |
| 0x8D | Print Screen |
| 0x8E | Scroll Lock |
| 0x8F | Pause |
| 0x90 | Insert |
| 0x91 | Home |
| 0x92 | Page Up |
| 0x93 | Delete |
| 0x94 | End |
| 0x95 | Page Down |
| 0x96 | Right Arrow |
| 0x97 | Left Arrow |
| 0x98 | Down Arrow |
| 0x99 | Up Arrow |

Host libraries that understand this extension can decode these codes
directly. Legacy BBQ20 hosts will see values ≥ 0x80 and can treat them
as unknown/unmapped keys.

See [HARDWARE.md](HARDWARE.md) for wiring details.
