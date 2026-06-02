/*
 * ESP8266 I2C Receiver for BT2I2C Bridge
 *
 * Polls key events from the Pico W BT2I2C bridge via I2C using the
 * official BBQ10Keyboard library (i2c_puppet protocol).
 *
 * Hardware Connections:
 * - BBQ20 GP28 (SDA) -> ESP8266 GPIO12 (D6)
 * - BBQ20 GP29 (SCL) -> ESP8266 GPIO14 (D5)
 * - BBQ20 GND        -> ESP8266 GND
 *
 * Dependencies:
 * - BBQ10Keyboard library: https://github.com/solderparty/arduino_bbq10kbd
 *   Install via Arduino Library Manager: search "BBQ10Keyboard"
 */

#include <Arduino.h>
#include <Wire.h>
#include <BBQ10Keyboard.h>

BBQ10Keyboard keyboard;

static char hid_to_char(uint8_t code)
{
    // Letters
    if (code >= 0x04 && code <= 0x1D)
        return 'a' + code - 0x04;
    // Numbers
    if (code >= 0x1E && code <= 0x27)
        return "1234567890"[code - 0x1E];
    // Common symbols and controls
    switch (code) {
        case 0x28: return '\n';
        case 0x2A: return '\b';
        case 0x2B: return '\t';
        case 0x2C: return ' ';
        case 0x2D: return '-';
        case 0x2E: return '=';
        case 0x2F: return '[';
        case 0x30: return ']';
        case 0x31: return '\\';
        case 0x33: return ';';
        case 0x34: return '\'';
        case 0x35: return '`';
        case 0x36: return ',';
        case 0x37: return '.';
        case 0x38: return '/';
        default:   return 0;
    }
}

static const char* key_name(char key)
{
    switch ((uint8_t)key) {
        case 0x00: return "None";
        case 0x04 ... 0x1D: {
            static const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            static char buf[2] = {0, 0};
            buf[0] = letters[key - 0x04];
            return buf;
        }
        case 0x1E ... 0x27: {
            static const char numbers[] = "1234567890";
            static char buf[2] = {0, 0};
            buf[0] = numbers[key - 0x1E];
            return buf;
        }
        case 0x28: return "Enter";
        case 0x29: return "Escape";
        case 0x2A: return "Backspace";
        case 0x2B: return "Tab";
        case 0x2C: return "Space";
        case 0x2D: return "-";
        case 0x2E: return "=";
        case 0x2F: return "[";
        case 0x30: return "]";
        case 0x31: return "\\";
        case 0x33: return ";";
        case 0x34: return "'";
        case 0x35: return "`";
        case 0x36: return ",";
        case 0x37: return ".";
        case 0x38: return "/";
        case 0x39: return "CapsLock";
        case 0x3A ... 0x45: {
            static const char* fkeys[] = {
                "F1","F2","F3","F4","F5","F6",
                "F7","F8","F9","F10","F11","F12"
            };
            return fkeys[key - 0x3A];
        }
        case 0x4C: return "Delete";
        case 0x9A: return "Alt";
        case 0x9B: return "LShift";
        case 0x9C: return "RShift";
        case 0x9D: return "Sym";
        default: return "Unknown";
    }
}

static void scan_i2c()
{
    Serial.println("Scanning I2C bus...");
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        uint8_t err = Wire.endTransmission();
        if (err == 0) {
            Serial.printf("  Found device at 0x%02X\n", addr);
        }
    }
    Serial.println("Scan done.");
}

void setup()
{
    Serial.begin(115200);
    while (!Serial && millis() < 3000);

    Serial.println();
    Serial.println("=== ESP8266 BT2I2C Receiver ===");
    Serial.println();

    Wire.begin(12, 14);  // SDA=GPIO12(D6), SCL=GPIO14(D5)
    Wire.setClock(100000L);

    scan_i2c();

    Serial.println("Initializing keyboard...");
    keyboard.begin();

    Serial.println("Reading version register...");
    uint8_t ver = keyboard.readRegister8(0x01);
    Serial.printf("REG_VER = 0x%02X\n", ver);
    if (ver == 0 || ver == 0xFF) {
        Serial.println("WARNING: version looks wrong - trying raw I2C read...");
        Wire.beginTransmission(0x1F);
        Wire.write(0x01);
        uint8_t err = Wire.endTransmission(false);
        Serial.printf("  write err=%u\n", err);
        uint8_t n = Wire.requestFrom((uint8_t)0x1F, (uint8_t)1);
        Serial.printf("  requested %u bytes, got %u\n", 1, n);
        if (n >= 1) {
            uint8_t raw = Wire.read();
            Serial.printf("  raw REG_VER = 0x%02X\n", raw);
            ver = raw;
        }
    }
    if (ver != 0 && ver != 0xFF) {
        Serial.printf("BT2I2C firmware v%d.%d detected\n", ver >> 4, ver & 0x0F);
    } else {
        Serial.println("ERROR: no response from keyboard at 0x1F");
        Serial.println("Check wiring: BBQ20 GP28->ESP GPIO12, GP29->ESP GPIO14, GND->GND");
    }

    Serial.println("Ready! Press keys on your BLE keyboard.");
    Serial.println();
}

void loop()
{
    static unsigned long last_heartbeat = 0;
    unsigned long now = millis();
    if (now - last_heartbeat > 5000) {
        last_heartbeat = now;
        Serial.println("[alive]");
    }

    int count = keyboard.keyCount();
    if (count == 0) {
        delay(5);
        return;
    }

    for (int i = 0; i < count; i++) {
        BBQ10Keyboard::KeyEvent ev = keyboard.keyEvent();
        if (ev.state == BBQ10Keyboard::StateIdle)
            break;

        const char* sname;
        switch (ev.state) {
            case BBQ10Keyboard::StatePress:     sname = "PRESSED";  break;
            case BBQ10Keyboard::StateLongPress: sname = "HOLD";     break;
            case BBQ10Keyboard::StateRelease:   sname = "RELEASED"; break;
            default:                            sname = "?";        break;
        }

        const char* name = key_name(ev.key);
        char ch = hid_to_char((uint8_t)ev.key);
        if (ch && ev.state == BBQ10Keyboard::StatePress) {
            Serial.print(ch);
        } else {
            Serial.printf("KEY %s: %s (0x%02x)", sname, name, (uint8_t)ev.key);
            if (ch) Serial.printf(" '%c'", ch);
            Serial.println();
        }
    }
}
