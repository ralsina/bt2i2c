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
    uint8_t write_buffer[2];
    uint8_t write_len;
} self;

static void irq_handler(void)
{
    // The controller sent data
    if (self.i2c->hw->intr_stat & I2C_IC_INTR_MASK_M_RX_FULL_BITS) {
        if (self.read_buffer.reg == REG_ID_INVALID) {
            self.read_buffer.reg = self.i2c->hw->data_cmd & 0xff;

            if (self.read_buffer.reg & PACKET_WRITE_MASK) {
                // It's a reg write, we need to wait for the second byte before we process
                return;
            }
        } else {
            self.read_buffer.data = self.i2c->hw->data_cmd & 0xff;
        }

        reg_process_packet(
            self.read_buffer.reg,
            self.read_buffer.data,
            self.write_buffer,
            &self.write_len);

        // Ready for the next operation
        self.read_buffer.reg = REG_ID_INVALID;
        return;
    }

    // The controller requested a read
    if (self.i2c->hw->intr_stat & I2C_IC_INTR_MASK_M_RD_REQ_BITS) {
        i2c_write_raw_blocking(self.i2c, self.write_buffer, self.write_len);
        self.i2c->hw->clr_rd_req;
        return;
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
    self.write_len = 0;

    // IRQ when the controller sends data, and when it requests a read
    self.i2c->hw->intr_mask = I2C_IC_INTR_MASK_M_RD_REQ_BITS
                            | I2C_IC_INTR_MASK_M_RX_FULL_BITS;

    const int irq = I2C0_IRQ + i2c_hw_index(self.i2c);
    irq_set_exclusive_handler(irq, irq_handler);
    irq_set_enabled(irq, true);

    printf("I2C slave initialized: GP%d(SDA)/GP%d(SCL), addr=0x%02X\n",
           PIN_I2C_SDA, PIN_I2C_SCL, reg_get_value(REG_ID_ADR));
}
