#pragma once

#include <stdint.h>
#include <stdbool.h>

enum key_state
{
    KEY_STATE_IDLE = 0,
    KEY_STATE_PRESSED,
    KEY_STATE_HOLD,
    KEY_STATE_RELEASED,
};

#define KEY_MOD_ALT         0x1A
#define KEY_MOD_SHL         0x1B
#define KEY_MOD_SHR         0x1C
#define KEY_MOD_SYM         0x1D

// Extended key codes (0x80+) — protocol extension beyond BBQ20.
// These are used for keys that have no printable ASCII representation
// and whose HID usage IDs would collide with the printable range.
#define KEY_EXT_ESC         0x80
#define KEY_EXT_F1          0x81
#define KEY_EXT_F2          0x82
#define KEY_EXT_F3          0x83
#define KEY_EXT_F4          0x84
#define KEY_EXT_F5          0x85
#define KEY_EXT_F6          0x86
#define KEY_EXT_F7          0x87
#define KEY_EXT_F8          0x88
#define KEY_EXT_F9          0x89
#define KEY_EXT_F10         0x8A
#define KEY_EXT_F11         0x8B
#define KEY_EXT_F12         0x8C
#define KEY_EXT_PRTSCR      0x8D
#define KEY_EXT_SCRLK       0x8E
#define KEY_EXT_PAUSE       0x8F
#define KEY_EXT_INSERT      0x90
#define KEY_EXT_HOME        0x91
#define KEY_EXT_PGUP        0x92
#define KEY_EXT_DELETE      0x93
#define KEY_EXT_END         0x94
#define KEY_EXT_PGDN        0x95
#define KEY_EXT_RIGHT       0x96
#define KEY_EXT_LEFT        0x97
#define KEY_EXT_DOWN        0x98
#define KEY_EXT_UP          0x99

void bt_keyboard_init(void);
bool bt_keyboard_is_connected(void);
bool bt_keyboard_reconnect_if_needed(void);
void bt_keyboard_start_pairing(void);
const char* bt_keyboard_get_device_name(void);
void bt_keyboard_notify_classic_connected(const char *name);
void bt_keyboard_notify_classic_connecting(const char *name);
void bt_keyboard_notify_classic_disconnected(void);
void bt_keyboard_classic_report(const uint8_t *report, uint16_t len);

int btstack_main(int argc, const char *argv[]);
