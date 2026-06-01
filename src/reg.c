#include "reg.h"
#include "fifo.h"

#include <pico/stdlib.h>

#define VERSION_MAJOR 0
#define VERSION_MINOR 3
#define VER_VAL        ((VERSION_MAJOR << 4) | (VERSION_MINOR << 0))

static struct
{
    uint8_t regs[REG_ID_LAST];
} self;

void reg_process_packet(uint8_t in_reg, uint8_t in_data, uint8_t *out_buffer, uint8_t *out_len)
{
    const bool is_write = (in_reg & PACKET_WRITE_MASK);
    const uint8_t reg = (in_reg & ~PACKET_WRITE_MASK);

    *out_len = 0;

    switch (reg) {

    case REG_ID_CFG:
    case REG_ID_INT:
    case REG_ID_ADR:
    {
        if (is_write) {
            reg_set_value(reg, in_data);
        } else {
            out_buffer[0] = reg_get_value(reg);
            *out_len = 1;
        }
        break;
    }

    case REG_ID_VER:
        out_buffer[0] = VER_VAL;
        *out_len = 1;
        break;

    case REG_ID_KEY:
    {
        uint8_t val = fifo_count() & KEY_COUNT_MASK;
        if (reg_is_bit_set(REG_ID_CFG, CFG_USE_MODS)) {
            // Report modifiers as active (not tracking capslock/numlock here,
            // but this bit indicates we apply modifiers)
        }
        out_buffer[0] = val;
        *out_len = 1;
        break;
    }

    case REG_ID_FIF:
    {
        struct fifo_item item = fifo_dequeue();
        out_buffer[0] = (uint8_t)item.state;
        out_buffer[1] = (uint8_t)item.key;
        *out_len = 2;

        if (item.state == KEY_STATE_IDLE) {
            out_buffer[0] = 0;
            out_buffer[1] = 0;
        }
        break;
    }

    case REG_ID_RST:
        if (is_write) {
            // Ignore reset writes for safety
        }
        break;

    default:
        break;
    }
}

uint8_t reg_get_value(enum reg_id reg)
{
    if (reg >= REG_ID_LAST) return 0;
    return self.regs[reg];
}

void reg_set_value(enum reg_id reg, uint8_t value)
{
    if (reg >= REG_ID_LAST) return;
    self.regs[reg] = value;
}

bool reg_is_bit_set(enum reg_id reg, uint8_t bit)
{
    if (reg >= REG_ID_LAST) return false;
    return self.regs[reg] & bit;
}

void reg_set_bit(enum reg_id reg, uint8_t bit)
{
    if (reg >= REG_ID_LAST) return;
    self.regs[reg] |= bit;
}

void reg_clear_bit(enum reg_id reg, uint8_t bit)
{
    if (reg >= REG_ID_LAST) return;
    self.regs[reg] &= ~bit;
}

void reg_init(void)
{
    reg_set_value(REG_ID_CFG, CFG_KEY_INT | CFG_USE_MODS);
    reg_set_value(REG_ID_ADR, 0x1F);
}
