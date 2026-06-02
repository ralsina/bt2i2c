#include "reg.h"
#include "fifo.h"

#include <pico/stdlib.h>
#include <string.h>

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VER_VAL        ((VERSION_MAJOR << 4) | (VERSION_MINOR << 0))

static struct
{
    uint8_t regs[REG_ID_LAST];
} self;

void reg_init(void)
{
    memset(self.regs, 0, sizeof(self.regs));
    // Default configuration: enable key interrupts, use modifiers
    reg_set_value(REG_ID_CFG, CFG_KEY_INT | CFG_USE_MODS);
    // Default I2C address (BBQ20 standard)
    reg_set_value(REG_ID_ADR, 0x1F);
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
    return (self.regs[reg] & bit) != 0;
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

void reg_process_packet(uint8_t in_reg, uint8_t in_data, uint8_t *out_buffer, uint8_t *out_len)
{
    const bool is_write = (in_reg & PACKET_WRITE_MASK);
    const uint8_t reg = (in_reg & ~PACKET_WRITE_MASK);

    *out_len = 0;

    switch (reg)
    {
    case REG_ID_CFG:
    case REG_ID_INT:
    case REG_ID_ADR:
        if (is_write) {
            reg_set_value(reg, in_data);
            // Sync I2C address when REG_ID_ADR changes
            if (reg == REG_ID_ADR) {
                extern void i2c_slave_sync_address(void);
                i2c_slave_sync_address();
            }
        } else {
            out_buffer[0] = reg_get_value(reg);
            *out_len = 1;
        }
        break;

    case REG_ID_VER:
        out_buffer[0] = VER_VAL;
        *out_len = 1;
        break;

    case REG_ID_KEY:
        // Return FIFO count (lower 5 bits)
        {
            uint8_t count = fifo_count() & KEY_COUNT_MASK;
            out_buffer[0] = count;
            *out_len = 1;

            // Set interrupt bit if keys available
            if (count > 0) {
                reg_set_bit(REG_ID_INT, INT_KEY);
            } else {
                reg_clear_bit(REG_ID_INT, INT_KEY);
            }
        }
        break;

    case REG_ID_FIF:
        // Dequeue and return key event (2 bytes: state + keycode)
        {
            struct fifo_item item = fifo_dequeue();
            out_buffer[0] = (uint8_t)item.state;
            out_buffer[1] = (uint8_t)item.key;
            *out_len = 2;

            // Clear interrupt if FIFO is now empty
            if (fifo_count() == 0) {
                reg_clear_bit(REG_ID_INT, INT_KEY);
            }
        }
        break;

    case REG_ID_RST:
        // Reset writes are ignored for safety
        if (is_write) {
            // Could implement soft reset here if needed
        }
        break;

    default:
        // Unknown register - return nothing
        break;
    }
}
