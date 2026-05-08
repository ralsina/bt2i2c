/*
 * BT2I2C Bridge — ESP8266 I2C Master Host
 *
 * Uses BBQ10Keyboard library to poll the BBQ20 (i2c-puppet protocol slave)
 * and prints keystrokes over serial.
 *
 * Wiring (NodeMCU → Slave):
 *   D1 (GPIO5) ── SCL
 *   D2 (GPIO4) ── SDA
 *   GND        ── GND
 *   (each board powered from its own USB)
 */

#include <Wire.h>
#include <BBQ10Keyboard.h>

BBQ10Keyboard keyboard;

#define KEY_MOD_ALT  0x9A
#define KEY_MOD_SHL  0x9B
#define KEY_MOD_SHR  0x9C
#define KEY_MOD_SYM  0x9D

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\nBT2I2C Host (ESP8266)");

    Wire.begin(4, 5);
    Wire.setClock(10000);
    pinMode(4, INPUT_PULLUP);
    pinMode(5, INPUT_PULLUP);

    keyboard.begin(0x1F);
    keyboard.setBacklight(0.5f);
    Serial.println("Ready");
}

void loop()
{
    int count = keyboard.keyCount();
    if (count > 0) {
        for (int i = 0; i < count; i++) {
            BBQ10Keyboard::KeyEvent ev = keyboard.keyEvent();
            uint8_t k = (uint8_t)ev.key;
            uint8_t s = ev.state;
            Serial.printf("key: 0x%02x ", k);
            switch (s) {
                case BBQ10Keyboard::StatePress:    Serial.println("PRESS"); break;
                case BBQ10Keyboard::StateLongPress: Serial.println("LONG"); break;
                case BBQ10Keyboard::StateRelease:  Serial.println("REL");  break;
                default:                           Serial.println("IDLE"); break;
            }
        }
    }
    delay(5);
}
