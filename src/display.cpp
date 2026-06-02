#include "pins.h"
#include "display.h"
#include <lgfx_config.hpp>
#include <LovyanGFX.hpp>
#include <stdio.h>
#include <string.h>
#include <pico/stdlib.h>

static LGFX lcd;
static bool display_initialized = false;

// Log buffer for scrolling display
#define LOG_BUFFER_SIZE 12
#define LOG_LINE_LENGTH 40
static char log_buffer[LOG_BUFFER_SIZE][LOG_LINE_LENGTH + 1];
static int log_index = 0;

void display_init(void)
{
    printf("[DISPLAY] Initializing LovyanGFX...\n");

    lcd.init();
    lcd.setRotation(0);
    lcd.setPivot(0, 0);

    // Clear screen with dark background
    lcd.fillScreen(TFT_BLACK);

    // Show startup screen
    lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    lcd.setFont(&fonts::Font0);
    lcd.setCursor(0, 0);
    lcd.println("BT2I2C Bridge");
    lcd.println("Initializing...");
    lcd.println("LovyanGFX Ready!");

    display_initialized = true;
    printf("[DISPLAY] LovyanGFX initialized: %dx%d\n", lcd.width(), lcd.height());

    sleep_ms(2000);
    display_clear();
}

void display_clear(void)
{
    if (!display_initialized) return;
    lcd.fillScreen(TFT_BLACK);
}

static void redraw_log_screen(void)
{
    if (!display_initialized) return;

    display_clear();

    // Title bar
    lcd.fillRect(0, 0, lcd.width(), 24, TFT_BLUE);
    lcd.setTextColor(TFT_WHITE, TFT_BLUE);
    lcd.setCursor(4, 4);
    lcd.print("BT2I2C Log");

    // Draw log lines
    lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    lcd.setCursor(4, 30);

    int y = 30;
    for (int i = 0; i < LOG_BUFFER_SIZE; i++) {
        int idx = (log_index - 1 - i + LOG_BUFFER_SIZE) % LOG_BUFFER_SIZE;
        if (strlen(log_buffer[idx]) > 0) {
            lcd.setCursor(4, y);
            lcd.println(log_buffer[idx]);
            y += 18;
            if (y > lcd.height() - 20) break;
        }
    }
}

void display_log(const char *message)
{
    // Print to serial
    printf("[LOG] %s\n", message);

    if (!display_initialized) return;

    // Add to circular buffer
    strncpy(log_buffer[log_index], message, LOG_LINE_LENGTH);
    log_buffer[log_index][LOG_LINE_LENGTH] = '\0';

    log_index = (log_index + 1) % LOG_BUFFER_SIZE;

    // Redraw screen
    redraw_log_screen();
}

void display_show_scanning(void)
{
    display_log("SCANNING for keyboard...");
}

void display_show_connecting(const char *device_name)
{
    char msg[LOG_LINE_LENGTH];
    snprintf(msg, sizeof(msg), "CONNECTING: %s", device_name);
    display_log(msg);
}

void display_show_connected(const char *device_name)
{
    char msg[LOG_LINE_LENGTH];
    snprintf(msg, sizeof(msg), "CONNECTED: %s", device_name);
    display_log(msg);
}

void display_show_disconnected(void)
{
    display_log("DISCONNECTED - Rescanning...");
}

void display_show_key_event(char key, bool pressed)
{
    // Only show key press events, not releases
    if (pressed) {
        char msg[LOG_LINE_LENGTH];
        snprintf(msg, sizeof(msg), "Key: %c (0x%02x)", key, (uint8_t)key);
        display_log(msg);
    }
}

void display_show_message(const char *line1, const char *line2)
{
    if (!display_initialized) return;

    display_clear();

    // Show centered message
    lcd.setTextColor(TFT_CYAN, TFT_BLACK);
    lcd.setFont(&fonts::Font4);

    // Center line 1
    int32_t w = lcd.textWidth(line1);
    lcd.setCursor((lcd.width() - w) / 2, 80);
    lcd.println(line1);

    // Center line 2
    lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    lcd.setFont(&fonts::Font2);
    w = lcd.textWidth(line2);
    lcd.setCursor((lcd.width() - w) / 2, 130);
    lcd.println(line2);

    sleep_ms(2000);
    redraw_log_screen();
}

void display_update(void)
{
    // Can be used for animations
    // For now, display is updated on log events
}