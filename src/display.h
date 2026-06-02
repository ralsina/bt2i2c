#pragma once

#include <stdbool.h>

// Initialize the ST7789 display
void display_init(void);

// Update connection status display
void display_show_scanning(void);
void display_show_connecting(const char *device_name);
void display_show_connected(const char *device_name);
void display_show_disconnected(void);

// Show key event on display
void display_show_key_event(char key, bool pressed);

// Show message on display
void display_show_message(const char *line1, const char *line2);

// Clear display
void display_clear(void);

// Update display (call periodically in main loop)
void display_update(void);
