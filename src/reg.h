#pragma once

#include <stdbool.h>
#include <stdint.h>

// Register IDs (compatible with i2c_puppet/BBQ20 protocol)
enum reg_id
{
    REG_ID_VER = 0x01,     // Firmware version
    REG_ID_CFG = 0x02,     // Configuration
    REG_ID_INT = 0x03,     // Interrupt status
    REG_ID_KEY = 0x04,     // Key count/FIFO status
    REG_ID_RST = 0x08,     // Reset
    REG_ID_FIF = 0x09,     // FIFO data (key events)
    REG_ID_ADR = 0x12,     // I2C address
    REG_ID_LAST,
};

// Configuration register bits
#define CFG_KEY_INT       (1 << 4)  // Enable key interrupts
#define CFG_REPORT_MODS   (1 << 6)  // Report modifier state
#define CFG_USE_MODS      (1 << 7)  // Apply modifiers to keys

// Interrupt register bits
#define INT_OVERFLOW      (1 << 0)  // FIFO overflow
#define INT_KEY           (1 << 3)  // Key available

// Key count mask
#define KEY_COUNT_MASK    0x1F

// Packet write bit (OR with register addr for write operations)
#define PACKET_WRITE_MASK (1 << 7)

// Register access functions
void reg_init(void);
uint8_t reg_get_value(enum reg_id reg);
void reg_set_value(enum reg_id reg, uint8_t value);
bool reg_is_bit_set(enum reg_id reg, uint8_t bit);
void reg_set_bit(enum reg_id reg, uint8_t bit);
void reg_clear_bit(enum reg_id reg, uint8_t bit);

// I2C packet processing
void reg_process_packet(uint8_t in_reg, uint8_t in_data, uint8_t *out_buffer, uint8_t *out_len);
