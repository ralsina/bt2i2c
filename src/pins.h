#pragma once

// I2C pins to host (GP4 = SDA, GP5 = SCL) using I2C0
#define PIN_I2C_SDA         4
#define PIN_I2C_SCL         5
#define I2C_INST            i2c0

// Pico W on-board LED via CYW43 (only if using CYW43)
#ifdef CYW43_WL_GPIO_LED_PIN
#define PICO_W_LED CYW43_WL_GPIO_LED_PIN
#endif

// Simple LED pin for testing (GP25 is onboard LED on regular Pico)
#define TEST_LED_PIN 25

// Pairing button - connect GP22 to GND for momentary switch
#define PIN_PAIRING_BUTTON 22

// BOOTSSEL button - built-in button on Pico W (GPIO 21)
// Can be used as a general-purpose button when not in BOOTSEL mode
#define PIN_BOOTSEL 21

// ST7789 Display pins (SPI)
#define SPI_PORT      0   // Use SPI0 for display
#define PIN_LCD_SCLK  2   // GP2  (Pin 4)
#define PIN_LCD_MOSI  3   // GP3  (Pin 5)
#define PIN_LCD_CS   -1   // No CS pin on this module
#define PIN_LCD_DC    16  // GP16 (Pin 21) - FIXED!
#define PIN_LCD_RST  14  // GP14 (Pin 19) - FIXED!
#define PIN_LCD_BL   15  // Optional - leave disconnected
#define PIN_LCD_GND  18  // Separate GND pin for display
