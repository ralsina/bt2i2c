#include "i2c_slave.h"
#include "reg.h"
#include "pins.h"

#include <hardware/i2c.h>
#include <hardware/irq.h>
#include <hardware/gpio.h>
#include <pico/stdlib.h>
#include <stdio.h>

#define REG_ID_INVALID 0x00

static struct {
    i2c_inst_t *i2c;
    struct {
        uint8_t reg;
        uint8_t data;
    } read_buffer;
    uint8_t selected_reg;
    bool has_selected_reg;
    uint8_t write_buffer[2];
    uint8_t write_len;
} self;

static void irq_handler(void)
{
    uint32_t intr_stat = self.i2c->hw->intr_stat;
    if (intr_stat == 0) {
        return;
    }

    // Drain all received bytes for this transaction.
    while (self.i2c->hw->status & I2C_IC_STATUS_RFNE_BITS) {
        uint8_t rx_byte = self.i2c->hw->data_cmd & 0xff;

        if (self.read_buffer.reg == REG_ID_INVALID) {
            self.read_buffer.reg = rx_byte;

            if (self.read_buffer.reg & PACKET_WRITE_MASK) {
                // Register write: wait for the payload byte.
                continue;
            }

            // Register select for a following read request.
            self.selected_reg = self.read_buffer.reg;
            self.has_selected_reg = true;
            self.read_buffer.data = 0;
        } else {
            self.read_buffer.data = rx_byte;
        }

        reg_process_packet(
            self.read_buffer.reg,
            self.read_buffer.data,
            self.write_buffer,
            &self.write_len);

        // Ready for the next operation.
        self.read_buffer.reg = REG_ID_INVALID;
    }

    // The controller requested a read. This can happen in the same IRQ as RX_FULL.
    if (intr_stat & I2C_IC_INTR_MASK_M_RD_REQ_BITS) {
        // Some hosts issue separate register-select and read phases.
        if (self.write_len == 0 && self.has_selected_reg) {
            reg_process_packet(
                self.selected_reg,
                0,
                self.write_buffer,
                &self.write_len);
        }

        if (self.write_len == 0) {
            self.write_buffer[0] = 0;
            self.write_len = 1;
        }

        i2c_write_raw_blocking(self.i2c, self.write_buffer, self.write_len);
        (void)self.i2c->hw->clr_rd_req;
    }

    // Clear TX abort if any occurred to keep the peripheral responsive.
    if (intr_stat & I2C_IC_INTR_MASK_M_TX_ABRT_BITS) {
        (void)self.i2c->hw->clr_tx_abrt;
    }
}

void i2c_slave_sync_address(void)
{
    i2c_set_slave_mode(self.i2c, true, reg_get_value(REG_ID_ADR));
}

void i2c_slave_init(void)
{
    // Use i2c0 with pins GP4 (SDA) and GP5 (SCL)
    self.i2c = i2c0;

    i2c_init(self.i2c, 100 * 1000);
    i2c_slave_sync_address();

    gpio_set_function(PIN_I2C_SDA, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_I2C_SDA);

    gpio_set_function(PIN_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_I2C_SCL);

    // Initialize read buffer
    self.read_buffer.reg = REG_ID_INVALID;
    self.read_buffer.data = 0;
    self.selected_reg = REG_ID_INVALID;
    self.has_selected_reg = false;
    self.write_len = 0;

    // IRQ when the controller sends data, and when it requests a read
    self.i2c->hw->intr_mask = I2C_IC_INTR_MASK_M_RD_REQ_BITS
                            | I2C_IC_INTR_MASK_M_RX_FULL_BITS
                            | I2C_IC_INTR_MASK_M_TX_ABRT_BITS;

    const int irq = I2C0_IRQ + i2c_hw_index(self.i2c);
    irq_set_exclusive_handler(irq, irq_handler);
    irq_set_enabled(irq, true);

    printf("I2C slave initialized: GP%d(SDA)/GP%d(SCL), addr=0x%02X\n",
           PIN_I2C_SDA, PIN_I2C_SCL, reg_get_value(REG_ID_ADR));
}
