#pragma once

#include <stdbool.h>
#include <stdint.h>

enum reg_id
{
    REG_ID_VER = 0x01,
    REG_ID_CFG = 0x02,
    REG_ID_INT = 0x03,
    REG_ID_KEY = 0x04,
    REG_ID_RST = 0x08,
    REG_ID_FIF = 0x09,
    REG_ID_ADR = 0x12,
    REG_ID_LAST,
};

#define CFG_KEY_INT       (1 << 4)
#define CFG_REPORT_MODS   (1 << 6)
#define CFG_USE_MODS      (1 << 7)

#define INT_OVERFLOW      (1 << 0)
#define INT_KEY           (1 << 3)

#define KEY_COUNT_MASK    0x1F

#define PACKET_WRITE_MASK (1 << 7)

void reg_process_packet(uint8_t in_reg, uint8_t in_data, uint8_t *out_buffer, uint8_t *out_len);
uint8_t reg_get_value(enum reg_id reg);
void reg_set_value(enum reg_id reg, uint8_t value);
bool reg_is_bit_set(enum reg_id reg, uint8_t bit);
void reg_set_bit(enum reg_id reg, uint8_t bit);
void reg_clear_bit(enum reg_id reg, uint8_t bit);
void reg_init(void);
