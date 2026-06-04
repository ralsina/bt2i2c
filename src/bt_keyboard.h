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
