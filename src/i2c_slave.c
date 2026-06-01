#include "i2c_slave.h"
#include "reg.h"
#include "pins.h"

#include <hardware/i2c.h>
#include <hardware/irq.h>
#include <pico/stdlib.h>

#define REG_ID_INVALID 0x00

static struct
{
    i2c_inst_t *i2c;
    struct
    {
        uint8_t reg;
        uint8_t data;
    } read_buffer;
    uint8_t write_buffer[2];
    uint8_t write_len;
} self;

static void irq_handler(void)
{
    if (self.i2c->hw->intr_stat & I2C_IC_INTR_MASK_M_RX_FULL_BITS) {
        if (self.read_buffer.reg == REG_ID_INVALID) {
            self.read_buffer.reg = self.i2c->hw->data_cmd & 0xff;
            if (self.read_buffer.reg & PACKET_WRITE_MASK) {
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

        self.read_buffer.reg = REG_ID_INVALID;
        return;
    }

    if (self.i2c->hw->intr_stat & I2C_IC_INTR_MASK_M_RD_REQ_BITS) {
        i2c_write_raw_blocking(self.i2c, self.write_buffer, self.write_len);
        self.i2c->hw->clr_rd_req;
        return;
    }
}

void i2c_slave_init(void)
{
    self.i2c = I2C_INST;
    self.read_buffer.reg = REG_ID_INVALID;

    i2c_init(self.i2c, 100 * 1000);
    i2c_set_slave_mode(self.i2c, true, reg_get_value(REG_ID_ADR));

    gpio_set_function(PIN_I2C_SDA, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_I2C_SDA);
    gpio_set_function(PIN_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_I2C_SCL);

    self.i2c->hw->intr_mask = I2C_IC_INTR_MASK_M_RD_REQ_BITS
                            | I2C_IC_INTR_MASK_M_RX_FULL_BITS;

    const int irq = I2C0_IRQ + i2c_hw_index(self.i2c);
    irq_set_exclusive_handler(irq, irq_handler);
    irq_set_enabled(irq, true);
}
