#include "pins.h"
#include "display.h"
#include "font8x8.h"

#include <hardware/spi.h>
#include <hardware/gpio.h>
#include <pico/stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


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
#define COLOR_LIGHTBLUE 0x3DFF

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
    uint8_t cb[2] = { (color >> 8) & 0xFF, color & 0xFF };
    for (size_t i = 0; i < pixel_count; i++) {
        spi_write_blocking(SPI_INST, cb, 2);
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
        cursor_x += 9 * scale;
    }
}

static int center_text_x(const char *text, int scale) {
    int len = 0;
    while (text[len]) len++;
    int width = len * 9 * scale;
    int x = (LCD_WIDTH - width) / 2;
    return x < 0 ? 0 : x;
}

static void fill_rect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
    lcd_set_window(x0, y0, x1, y1);
    size_t count = (size_t)(x1 - x0 + 1) * (y1 - y0 + 1);
    lcd_fill_pixels(color, count);
}

void display_init(void) {
    printf("[DISPLAY] === DISPLAY INIT START ===\n");
    printf("[DISPLAY] Pin definitions:\n");
    printf("[DISPLAY]   SCK (Clock):  GP%d (Pin %d)\n", PIN_LCD_SCLK, 4);
    printf("[DISPLAY]   MOSI (Data):  GP%d (Pin %d)\n", PIN_LCD_MOSI, 5);
    printf("[DISPLAY]   DC (Cmd/Data): GP%d (Pin %d)\n", PIN_LCD_DC, 21);
    printf("[DISPLAY]   RES (Reset):  GP%d (Pin %d)\n", PIN_LCD_RST, 19);
    printf("[DISPLAY]   GND:          Pin %d\n", 18);
    printf("[DISPLAY]   VCC:          Pin %d\n", 40);

    printf("[DISPLAY] Setting GND pin...\n");
    gpio_init(PIN_LCD_GND);
    gpio_set_dir(PIN_LCD_GND, GPIO_OUT);
    gpio_put(PIN_LCD_GND, 0);

    printf("[DISPLAY] Initializing control pins first...\n");

    gpio_init(PIN_LCD_DC);
    gpio_set_dir(PIN_LCD_DC, GPIO_OUT);
    gpio_put(PIN_LCD_DC, 0);

    if (PIN_LCD_RST >= 0) {
        gpio_init(PIN_LCD_RST);
        gpio_set_dir(PIN_LCD_RST, GPIO_OUT);
        gpio_put(PIN_LCD_RST, 1);
    }

    if (PIN_LCD_CS >= 0) {
        gpio_init(PIN_LCD_CS);
        gpio_set_dir(PIN_LCD_CS, GPIO_OUT);
        gpio_put(PIN_LCD_CS, 1);
    } else {
        printf("[DISPLAY] No CS pin - display always listening\n");
    }

    if (PIN_LCD_BL >= 0) {
        gpio_init(PIN_LCD_BL);
        gpio_set_dir(PIN_LCD_BL, GPIO_OUT);
        gpio_put(PIN_LCD_BL, 1);
    }

    printf("[DISPLAY] Initializing SPI at 16MHz, Mode 3, 8-bit...\n");
    spi_init(SPI_INST, 16 * 1000 * 1000);

    spi_set_format(SPI_INST, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);

    gpio_set_function(PIN_LCD_SCLK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_LCD_MOSI, GPIO_FUNC_SPI);
    printf("[DISPLAY] SPI configured: SCK=GP%d, MOSI=GP%d\n", PIN_LCD_SCLK, PIN_LCD_MOSI);

    lcd_init_sequence();

    printf("Display initialized: %dx%d SPI LCD\n", LCD_WIDTH, LCD_HEIGHT);
    printf("[DISPLAY] === DISPLAY INIT COMPLETE ===\n");
}

void display_clear(void) {
    lcd_set_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    lcd_fill_pixels(COLOR_BLACK, LCD_WIDTH * LCD_HEIGHT);
}

#define TITLE_BAR_HEIGHT 44
#define TITLE_TEXT_Y 12
#define INFO_START_Y 60
#define INFO_LINE_SPACING 16
#define INFO_LEFT_MARGIN 20

static void draw_status_screen(const char *title, const char *detail1, const char *detail2, const char *detail3, uint16_t bar_color) {
    fill_rect(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, COLOR_BLACK);

    fill_rect(0, 0, LCD_WIDTH - 1, TITLE_BAR_HEIGHT - 1, bar_color);

    uint8_t black[2] = {0x00, 0x00};
    uint8_t white[2] = {0xFF, 0xFF};
    uint8_t bar_cb[2] = { (bar_color >> 8) & 0xFF, bar_color & 0xFF };
    draw_text(title, center_text_x(title, 2), TITLE_TEXT_Y, black, bar_cb, 2);

    int y = INFO_START_Y;
    if (detail1) {
        draw_text(detail1, INFO_LEFT_MARGIN, y, white, black, 1);
        y += INFO_LINE_SPACING;
    }
    if (detail2) {
        draw_text(detail2, INFO_LEFT_MARGIN, y, white, black, 1);
        y += INFO_LINE_SPACING;
    }
    if (detail3) {
        draw_text(detail3, INFO_LEFT_MARGIN, y, white, black, 1);
    }
}

void display_show_scanning(void) {
    draw_status_screen("SCANNING", "Searching for", "keyboard...", "Press btn to pair", COLOR_CYAN);
    printf("[DISPLAY] Showing: SCANNING\n");
}

void display_show_connecting(const char *device_name) {
    draw_status_screen("CONNECTING", device_name, NULL, NULL, COLOR_YELLOW);
    printf("[DISPLAY] Showing: CONNECTING to %s\n", device_name);
}

void display_show_connected(const char *device_name) {
    draw_status_screen("CONNECTED", device_name, NULL, NULL, COLOR_GREEN);
    printf("[DISPLAY] Showing: CONNECTED to %s\n", device_name);
}

void display_show_disconnected(void) {
    draw_status_screen("LOST", "Reconnecting...", NULL, NULL, COLOR_RED);
    printf("[DISPLAY] Showing: DISCONNECTED\n");
}

void display_show_key_event(char key, bool pressed) {
    if (pressed) {
        fill_rect(0, 160, LCD_WIDTH - 1, 199, COLOR_YELLOW);
        sleep_ms(50);
        fill_rect(0, 160, LCD_WIDTH - 1, 199, COLOR_BLACK);
    }
}

void display_show_message(const char *line1, const char *line2) {
    draw_status_screen(line1, line2, NULL, NULL, COLOR_LIGHTBLUE);
    printf("[DISPLAY] Message: %s / %s\n", line1, line2);
}

void display_update(void) {
}

#define LOG_BUFFER_SIZE 10
#define LOG_LINE_LENGTH 30
static char *log_buffer[LOG_BUFFER_SIZE] = {0};
static int log_index = 0;

void display_log(const char *message) {
    printf("[LOG] %s\n", message);

    if (log_buffer[log_index]) {
        free(log_buffer[log_index]);
    }

    log_buffer[log_index] = malloc(LOG_LINE_LENGTH + 1);
    if (log_buffer[log_index]) {
        strncpy(log_buffer[log_index], message, LOG_LINE_LENGTH);
        log_buffer[log_index][LOG_LINE_LENGTH] = '\0';
    }

    log_index = (log_index + 1) % LOG_BUFFER_SIZE;
}
