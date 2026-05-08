#pragma once

#include <stdbool.h>
#include <stdint.h>

enum reg_id
{
    REG_ID_VER = 0x01,
    REG_ID_CFG = 0x02,
    REG_ID_INT = 0x03,
    REG_ID_KEY = 0x04,
    REG_ID_BKL = 0x05,
    REG_ID_DEB = 0x06,
    REG_ID_FRQ = 0x07,
    REG_ID_RST = 0x08,
    REG_ID_FIF = 0x09,
    REG_ID_BK2 = 0x0A,
    REG_ID_DIR = 0x0B,
    REG_ID_PUE = 0x0C,
    REG_ID_PUD = 0x0D,
    REG_ID_GIO = 0x0E,
    REG_ID_GIC = 0x0F,
    REG_ID_GIN = 0x10,
    REG_ID_HLD = 0x11,
    REG_ID_ADR = 0x12,
    REG_ID_IND = 0x13,
    REG_ID_CF2 = 0x14,
    REG_ID_TOX = 0x15,
    REG_ID_TOY = 0x16,

    REG_ID_LAST,
};

#define CFG_OVERFLOW_ON     (1 << 0)
#define CFG_OVERFLOW_INT    (1 << 1)
#define CFG_CAPSLOCK_INT    (1 << 2)
#define CFG_NUMLOCK_INT     (1 << 3)
#define CFG_KEY_INT         (1 << 4)
#define CFG_PANIC_INT       (1 << 5)
#define CFG_REPORT_MODS     (1 << 6)
#define CFG_USE_MODS        (1 << 7)

#define CF2_TOUCH_INT       (1 << 0)
#define CF2_USB_KEYB_ON     (1 << 1)
#define CF2_USB_MOUSE_ON    (1 << 2)

#define INT_OVERFLOW        (1 << 0)
#define INT_CAPSLOCK        (1 << 1)
#define INT_NUMLOCK         (1 << 2)
#define INT_KEY             (1 << 3)
#define INT_PANIC           (1 << 4)
#define INT_GPIO            (1 << 5)
#define INT_TOUCH           (1 << 6)

#define KEY_CAPSLOCK        (1 << 5)
#define KEY_NUMLOCK         (1 << 6)
#define KEY_COUNT_MASK      0x1F

#define DIR_OUTPUT          0
#define DIR_INPUT           1

#define PUD_DOWN            0
#define PUD_UP              1

#define VER_MAJOR           1
#define VER_MINOR           0
#define VER_VAL             ((VER_MAJOR << 4) | (VER_MINOR << 0))

#define PACKET_WRITE_MASK   (1 << 7)

// Key codes used in the i2c_puppet protocol
#define KEY_MOD_ALT         0x9A
#define KEY_MOD_SHL         0x9B
#define KEY_MOD_SHR         0x9C
#define KEY_MOD_SYM         0x9D

enum key_state
{
    KEY_STATE_IDLE = 0,
    KEY_STATE_PRESSED,
    KEY_STATE_HOLD,
    KEY_STATE_RELEASED,
};

void reg_process_packet(uint8_t in_reg, uint8_t in_data,
                        uint8_t *out_buffer, uint8_t *out_len);

uint8_t reg_get_value(enum reg_id reg);
void reg_set_value(enum reg_id reg, uint8_t value);

bool reg_is_bit_set(enum reg_id reg, uint8_t bit);
void reg_set_bit(enum reg_id reg, uint8_t bit);
void reg_clear_bit(enum reg_id reg, uint8_t bit);

void reg_init(void);
