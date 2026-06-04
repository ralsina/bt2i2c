#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <btstack.h>

void bt_classic_init(void);
void bt_classic_start_inquiry(void);
void bt_classic_stop_inquiry(void);
void bt_classic_connect(bd_addr_t addr);
void bt_classic_disconnect(void);
bool bt_classic_is_connected(void);
const char *bt_classic_get_device_name(void);
void bt_classic_set_device_name(const uint8_t *name, uint8_t name_len);
