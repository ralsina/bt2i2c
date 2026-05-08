#pragma once

#include <stdint.h>
#include <stdbool.h>

void bt_keyboard_init(void);
bool bt_keyboard_is_connected(void);

int btstack_main(int argc, const char *argv[]);
