#include "pins.h"
#include "display.h"
#include "font8x8.h"

#include <hardware/spi.h>
#include <hardware/gpio.h>
#include <pico/stdlib.h>
#include <string.h>
#include <stdio.h>


// Display dimensions
#define LCD_WIDTH  240
#define LCD_HEIGHT 240

// Round display settings
#define ROUND_DISPLAY_OFFSET_X 0
#define ROUND_DISPLAY_OFFSET_Y 0

// ST7789 Commands
#define ST7789_NOP      0x00
#define ST7789_SWRESET  0x01
#define ST7789_RDDID    0x04
#define ST7789_RDDST    0x09
#define ST7789_SLPIN    0x10
#define ST7789_SLPOUT   0x11
#define ST7789_PTLON    0x12
#define ST7789_NORON    0x13
#define ST7789_INVOFF   0x20
#define ST7789_INVON    0x21
#define ST7789_DISPOFF  0x28
#define ST7789_DISPON   0x29
#define ST7789_CASET    0x2A
#define ST7789_RASET    0x2B
#define ST7789_RAMWR    0x2C
#define ST7789_RAMRD    0x2E
#define ST7789_PTLAR    0x30
#define ST7789_COLMOD   0x3A
#define ST7789_MADCTL   0x36
#define ST7789_FRMCTR1  0xB1
#define ST7789_FRMCTR2  0xB2
#define ST7789_FRMCTR3  0xB3
#define ST7789_INVCTR   0xB4
#define ST7789_DISSET5  0xB6
#define ST7789_PWCTR1   0xC0
#define ST7789_PWCTR2   0xC1
#define ST7789_PWCTR3   0xC2
#define ST7789_PWCTR4   0xC3
#define ST7789_PWCTR5   0xC4
#define ST7789_VMCTR1   0xC5
#define ST7789_RDID1    0xDA
#define ST7789_RDID2    0xDB
#define ST7789_RDID3    0xDC
#define ST7789_RDID4    0xDD
#define ST7789_GMCTRP1  0xE0
#define ST7789_GMCTRN1  0xE1
#define ST7789_MADCTL_RGB 0x00

// SPI instance (use SPI0 for display)
#define SPI_INST spi0

static uint16_t frame_buffer[LCD_WIDTH * LCD_HEIGHT];

// Color definitions
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0
#define COLOR_CYAN    0x07FF
#define COLOR_MAGENTA 0xF81F
#define COLOR_ORANGE  0xFD20

static inline void cs_select(void) {
    if (PIN_LCD_CS >= 0) {
        gpio_put(PIN_LCD_CS, 0);
    }
}

static inline void cs_deselect(void) {
    if (PIN_LCD_CS >= 0) {
        gpio_put(PIN_LCD_CS, 1);
    }
}

static inline void dc_cmd(void) {
    gpio_put(PIN_LCD_DC, 0);
}

static inline void dc_data(void) {
    gpio_put(PIN_LCD_DC, 1);
}

static void write_cmd(uint8_t cmd) {
    cs_select();
    dc_cmd();
    spi_write_blocking(SPI_INST, &cmd, 1);
    cs_deselect();
}

static void write_data(const uint8_t *data, size_t len) {
    cs_select();
    dc_data();
    spi_write_blocking(SPI_INST, data, len);
    cs_deselect();
}

static void lcd_init_sequence_st7735(void) {
    printf("[DISPLAY] === Trying ST7735 initialization ===\n");

    // Reset
    printf("[DISPLAY] Hardware reset...\n");
    if (PIN_LCD_RST >= 0) {
        gpio_put(PIN_LCD_RST, 0);
        sleep_ms(10);
        gpio_put(PIN_LCD_RST, 1);
        sleep_ms(150);
    }

    // Software reset
    printf("[DISPLAY] Software reset...\n");
    write_cmd(ST7789_SWRESET);
    sleep_ms(150);

    // Sleep out
    printf("[DISPLAY] Sleep out...\n");
    write_cmd(ST7789_SLPOUT);
    sleep_ms(500);

    // Set color mode to 16-bit
    printf("[DISPLAY] Setting 16-bit color mode...\n");
    uint8_t colmod = 0x05; // 16-bit color for ST7735
    write_cmd(ST7789_COLMOD);
    write_data(&colmod, 1);

    // Set MADCTL for ST7735
    printf("[DISPLAY] Setting MADCTL for ST7735...\n");
    uint8_t madctl = 0xC0; // MADCTL for ST7735 (RGB + rotation)
    write_cmd(ST7789_MADCTL);
    write_data(&madctl, 1);

    // Normal display on
    printf("[DISPLAY] Normal display on...\n");
    write_cmd(ST7789_NORON);

    // Display on
    printf("[DISPLAY] Display on...\n");
    write_cmd(ST7789_DISPON);
    sleep_ms(100);

    printf("[DISPLAY] ST7735 initialization complete\n");
}

static void lcd_init_sequence_simple(void) {
    printf("[DISPLAY] === Trying SIMPLE ST7789 initialization ===\n");

    // Reset
    if (PIN_LCD_RST >= 0) {
        gpio_put(PIN_LCD_RST, 0);
        sleep_ms(10);
        gpio_put(PIN_LCD_RST, 1);
        sleep_ms(150);
    }

    // Software reset
    write_cmd(ST7789_SWRESET);
    sleep_ms(150);

    // Sleep out
    write_cmd(ST7789_SLPOUT);
    sleep_ms(500);

    // Simple color mode setting
    uint8_t colmod = 0x55; // 16-bit
    write_cmd(ST7789_COLMOD);
    write_data(&colmod, 1);

    // Simple MADCTL
    uint8_t madctl = 0x00; // No rotation, no RGB swap
    write_cmd(ST7789_MADCTL);
    write_data(&madctl, 1);

    // Display on
    write_cmd(ST7789_DISPON);
    sleep_ms(100);

    printf("[DISPLAY] Simple ST7789 initialization complete\n");
}

static void lcd_init_sequence(void) {
    printf("[DISPLAY] === ST7789 FULL INITIALIZATION ===\n");

    // CRITICAL: Proper hardware reset sequence to clear any stray data
    if (PIN_LCD_RST >= 0) {
        printf("[DISPLAY] Performing hardware reset...\n");
        gpio_put(PIN_LCD_RST, 1);
        sleep_ms(10);
        gpio_put(PIN_LCD_RST, 0); // Pull reset low to reset
        sleep_ms(50);
        gpio_put(PIN_LCD_RST, 1); // Bring it back high
        sleep_ms(120); // ESSENTIAL delay for ST7789 controller to stabilize
    }

    // Software reset
    printf("[DISPLAY] Software reset...\n");
    write_cmd(0x01); // SWRESET
    sleep_ms(150);

    // Sleep out
    printf("[DISPLAY] Sleep out...\n");
    write_cmd(0x11); // SLPOUT
    sleep_ms(200);

    // Color mode: 16-bit
    printf("[DISPLAY] Setting 16-bit color mode...\n");
    write_cmd(0x3A); // COLMOD
    uint8_t colmod = 0x55; // 16-bit color
    write_data(&colmod, 1);
    sleep_ms(50);

    // Memory Access Control (no rotation, RGB order)
    printf("[DISPLAY] Setting memory access control...\n");
    write_cmd(0x36); // MADCTL
    uint8_t madctl = 0x00; // Normal mode
    write_data(&madctl, 1);
    sleep_ms(50);

    // Normal display mode on
    printf("[DISPLAY] Normal display mode on...\n");
    write_cmd(0x13); // NORON
    sleep_ms(50);

    // Display inversion ON (fix inverted colors!)
    printf("[DISPLAY] Enabling display inversion...\n");
    write_cmd(0x21); // INVON
    sleep_ms(50);

    // Display on
    printf("[DISPLAY] Display on...\n");
    write_cmd(0x29); // DISPON
    sleep_ms(200);

    printf("[DISPLAY] ST7789 initialization complete\n");
}

static void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint8_t data[4];

    // Column address set
    write_cmd(ST7789_CASET);
    data[0] = x0 >> 8;
    data[1] = x0 & 0xFF;
    data[2] = x1 >> 8;
    data[3] = x1 & 0xFF;
    write_data(data, 4);

    // Row address set
    write_cmd(ST7789_RASET);
    data[0] = y0 >> 8;
    data[1] = y0 & 0xFF;
    data[2] = y1 >> 8;
    data[3] = y1 & 0xFF;
    write_data(data, 4);

    // Memory write
    write_cmd(ST7789_RAMWR);
}

static void lcd_fill_pixels(uint16_t color, size_t pixel_count) {
    cs_select();
    dc_data();

    // Try sending the SAME byte repeatedly to test
    uint8_t test_byte = 0xFF;  // Send all 1s (white in 565)
    for (size_t i = 0; i < pixel_count * 2; i++) {
        spi_write_blocking(SPI_INST, &test_byte, 1);
    }

    cs_deselect();
}

#include "font8x8.h"

static void draw_char_8x8(char c, int x, int y, uint8_t fg[2], uint8_t bg[2], int scale) {
    if (c < 0 || c >= 128 || font8x8[(int)c][0] == 0 && font8x8[(int)c][7] == 0) {
        c = ' '; // Default to space if not found
    }

    // Set window for character
    int char_width = 8 * scale;
    int char_height = 8 * scale;
    lcd_set_window(x, y, x + char_width - 1, y + char_height - 1);
    cs_select();
    dc_data();

    for (int row = 0; row < 8; row++) {
        uint8_t font_row = font8x8[(int)c][row];
        for (int row_scale = 0; row_scale < scale; row_scale++) {
            for (int col = 0; col < 8; col++) {
                uint8_t *color = (font_row & (0x80 >> col)) ? fg : bg;
                for (int col_scale = 0; col_scale < scale; col_scale++) {
                    spi_write_blocking(SPI_INST, color, 2);
                }
            }
        }
    }
    cs_deselect();
}

static void draw_text(const char *text, int x, int y, uint8_t fg[2], uint8_t bg[2], int scale) {
    int cursor_x = x;
    while (*text && cursor_x < LCD_WIDTH - 8 * scale) {
        draw_char_8x8(*text++, cursor_x, y, fg, bg, scale);
        cursor_x += 9 * scale; // 8 pixels + 1 space
    }
}

static void display_boot_animation(void) {
    printf("[DISPLAY] === TESTING 8x8 FONT LIBRARY ===\n");

    // White background
    printf("[DISPLAY] White background...\n");
    lcd_set_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    cs_select();
    dc_data();
    uint8_t white[2] = {0xFF, 0xFF};
    for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        spi_write_blocking(SPI_INST, white, 2);
    }
    cs_deselect();

    // Colors
    uint8_t black[2] = {0x00, 0x00};
    uint8_t red[2] = {0xF8, 0x00};

    // Draw "BT2I2C" in red
    printf("[DISPLAY] Drawing 'BT2I2C' with 8x8 font...\n");
    draw_text("BT2I2C", 50, 30, red, white, 3);

    // Draw "Ready" in black
    printf("[DISPLAY] Drawing 'Ready' with 8x8 font...\n");
    draw_text("Ready", 80, 100, black, white, 2);

    printf("[DISPLAY] Holding for 5 seconds...\n");
    sleep_ms(5000);

    printf("[DISPLAY] Font library test complete!\n");
}

void display_init(void) {
    printf("[DISPLAY] === DISPLAY INIT START ===\n");
    printf("[DISPLAY] Pin definitions:\n");
    printf("[DISPLAY]   SCK (Clock):  GP%d (Pin %d)\n", PIN_LCD_SCK, 4);
    printf("[DISPLAY]   MOSI (Data):  GP%d (Pin %d)\n", PIN_LCD_MOSI, 5);
    printf("[DISPLAY]   DC (Cmd/Data): GP%d (Pin %d)\n", PIN_LCD_DC, 21);
    printf("[DISPLAY]   RES (Reset):  GP%d (Pin %d)\n", PIN_LCD_RST, 19);
    printf("[DISPLAY]   GND:          Pin %d\n", 18);
    printf("[DISPLAY]   VCC:          Pin %d\n", 40);

    // Initialize GND pin for display
    printf("[DISPLAY] Setting GND pin...\n");
    gpio_init(PIN_LCD_GND);
    gpio_set_dir(PIN_LCD_GND, GPIO_OUT);
    gpio_put(PIN_LCD_GND, 0); // Set to GND

    // CRITICAL: Initialize control pins BEFORE SPI to avoid stray data
    printf("[DISPLAY] Initializing control pins first...\n");

    gpio_init(PIN_LCD_DC);
    gpio_set_dir(PIN_LCD_DC, GPIO_OUT);
    gpio_put(PIN_LCD_DC, 0);

    if (PIN_LCD_RST >= 0) {
        gpio_init(PIN_LCD_RST);
        gpio_set_dir(PIN_LCD_RST, GPIO_OUT);
        gpio_put(PIN_LCD_RST, 1);
    }

    // CS pin (optional - some modules don't have CS)
    if (PIN_LCD_CS >= 0) {
        gpio_init(PIN_LCD_CS);
        gpio_set_dir(PIN_LCD_CS, GPIO_OUT);
        gpio_put(PIN_LCD_CS, 1); // Deselect
    } else {
        printf("[DISPLAY] No CS pin - display always listening\n");
    }

    // Initialize backlight (optional)
    if (PIN_LCD_BL >= 0) {
        gpio_init(PIN_LCD_BL);
        gpio_set_dir(PIN_LCD_BL, GPIO_OUT);
        gpio_put(PIN_LCD_BL, 1);
    }

    // NOW initialize SPI (after control pins to prevent stray data)
    printf("[DISPLAY] Initializing SPI at 16MHz, Mode 3, 8-bit...\n");
    spi_init(SPI_INST, 16 * 1000 * 1000); // 16MHz safe speed

    // CRITICAL: Set to Mode 3 BEFORE setting GPIO functions
    spi_set_format(SPI_INST, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);

    // NOW assign SPI functions to GPIO pins
    gpio_set_function(PIN_LCD_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_LCD_MOSI, GPIO_FUNC_SPI);
    printf("[DISPLAY] SPI configured: SCK=GP%d, MOSI=GP%d\n", PIN_LCD_SCK, PIN_LCD_MOSI);

    // Initialize display
    lcd_init_sequence();

    printf("Display initialized: %dx%d SPI LCD\n", LCD_WIDTH, LCD_HEIGHT);

    // Run very visible boot animation
    printf("Running display boot animation...\n");
    display_boot_animation();
    printf("Boot animation complete!\n");
    printf("[DISPLAY] === DISPLAY INIT COMPLETE ===\n");
}

void display_clear(void) {
    lcd_set_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    lcd_fill_pixels(COLOR_BLACK, LCD_WIDTH * LCD_HEIGHT);
}

static void draw_status_screen(const char *status, const char *detail, uint16_t color) {
    display_clear();

    // Draw status bar at top (color indicates state)
    lcd_set_window(0, 0, LCD_WIDTH - 1, 40);
    lcd_fill_pixels(color, LCD_WIDTH * 40);

    // Draw detail text area (white on black)
    // In a full implementation, this would render actual text
    // For now, we use colored regions to indicate status

    // Status indicator bar
    lcd_set_window(0, 80, LCD_WIDTH - 1, 160);
    lcd_fill_pixels(COLOR_WHITE, LCD_WIDTH * 80);

    // Bottom bar (blue)
    lcd_set_window(0, 200, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    lcd_fill_pixels(COLOR_BLUE, LCD_WIDTH * 40);
}

void display_show_scanning(void) {
    draw_status_screen("SCANNING", "Searching for keyboard...", COLOR_CYAN);
    printf("[DISPLAY] Showing: SCANNING\n");
}

void display_show_connecting(const char *device_name) {
    draw_status_screen("CONNECTING", device_name, COLOR_YELLOW);
    printf("[DISPLAY] Showing: CONNECTING to %s\n", device_name);
}

void display_show_connected(const char *device_name) {
    draw_status_screen("CONNECTED", device_name, COLOR_GREEN);
    printf("[DISPLAY] Showing: CONNECTED to %s\n", device_name);
}

void display_show_disconnected(void) {
    draw_status_screen("DISCONNECTED", "Keyboard lost", COLOR_RED);
    printf("[DISPLAY] Showing: DISCONNECTED\n");
}

void display_show_key_event(char key, bool pressed) {
    // Flash screen briefly on key press
    if (pressed) {
        lcd_set_window(0, 160, LCD_WIDTH - 1, 200);
        lcd_fill_pixels(COLOR_YELLOW, LCD_WIDTH * 40);
        sleep_ms(50);
        lcd_set_window(0, 160, LCD_WIDTH - 1, 200);
        lcd_fill_pixels(COLOR_WHITE, LCD_WIDTH * 40);
    }
}

void display_show_message(const char *line1, const char *line2) {
    draw_status_screen(line1, line2, COLOR_MAGENTA);
    printf("[DISPLAY] Message: %s / %s\n", line1, line2);
}

void display_update(void) {
    // Can be used for animations or periodic updates
    // For now, display is static between status changes
}
