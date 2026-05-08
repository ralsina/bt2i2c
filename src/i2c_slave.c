#include "i2c_slave.h"
#include "reg.h"
#include "pins.h"

#include <hardware/i2c.h>
#include <hardware/irq.h>
#include <pico/stdlib.h>
#include <stdio.h>

#define REG_ID_INVALID 0x00

static i2c_inst_t *i2c_instances[2] = { i2c0, i2c1 };

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

            if (self.read_buffer.reg & PACKET_WRITE_MASK)
                return;
        } else {
            self.read_buffer.data = self.i2c->hw->data_cmd & 0xff;
        }

        reg_process_packet(
            self.read_buffer.reg, self.read_buffer.data,
            self.write_buffer, &self.write_len);

        self.read_buffer.reg = REG_ID_INVALID;
        return;
    }

    if (self.i2c->hw->intr_stat & I2C_IC_INTR_MASK_M_RD_REQ_BITS) {
        if (self.write_len > 0)
            i2c_write_raw_blocking(self.i2c, self.write_buffer, self.write_len);
        self.i2c->hw->clr_rd_req;
        return;
    }

    if (self.i2c->hw->intr_stat & I2C_IC_INTR_MASK_M_TX_ABRT_BITS) {
        self.i2c->hw->clr_tx_abrt;
        self.read_buffer.reg = REG_ID_INVALID;
        return;
    }
}

void i2c_slave_sync_address(void)
{
    i2c_set_slave_mode(self.i2c, true, reg_get_value(REG_ID_ADR));
}

void i2c_slave_init(void)
{
    int irq_num;

    printf("i2c_slave_init: start\n");
    self.i2c = i2c_instances[(PIN_PUPPET_SCL / 2) % 2];
    self.read_buffer.reg = REG_ID_INVALID;

    // Set GPIO function BEFORE i2c_init so the I2C peripheral sees proper bus state
    printf("i2c_slave_init: setting GPIO function\n");
    gpio_set_function(PIN_PUPPET_SDA, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_PUPPET_SDA);
    gpio_set_function(PIN_PUPPET_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_PUPPET_SCL);

    printf("i2c_slave_init: calling i2c_init\n");
    i2c_init(self.i2c, 100 * 1000);
    printf("i2c_slave_init: i2c_init done\n");
    i2c_slave_sync_address();
    printf("i2c_slave_init: sync_address done\n");

    // Clear any spurious interrupt conditions that may have occurred during init
    printf("i2c_slave_init: clearing spurious intr\n");
    self.i2c->hw->clr_tx_abrt;
    self.i2c->hw->clr_rd_req;
    self.i2c->hw->clr_stop_det;
    self.i2c->hw->clr_start_det;
    self.i2c->hw->clr_activity;
    self.i2c->hw->clr_gen_call;

    // INTR_MASK is active-low: write 0 to enable, 1 to mask
    printf("i2c_slave_init: intr_stat=0x%08lx\n",
           (unsigned long)self.i2c->hw->intr_stat);
    printf("i2c_slave_init: masking all periph interrupts\n");
    self.i2c->hw->intr_mask = 0xFF;
    printf("i2c_slave_init: setting irq handler\n");
    irq_num = I2C0_IRQ + i2c_hw_index(self.i2c);
    irq_set_exclusive_handler(irq_num, irq_handler);
    // Clear any pending bit the NVIC may have latched from during init
    printf("i2c_slave_init: clearing NVIC pending\n");
    irq_clear(irq_num);
    printf("i2c_slave_init: enabling NVIC IRQ\n");
    // Use IRQ directly instead of SDK wrapper
    uint32_t *iser = (uint32_t *)0xE000E100;
    *iser = 1u << (irq_num & 0x1F);
    printf("i2c_slave_init: NVIC IRQ enabled\n");

    // Now safely unmask the peripheral interrupts we want
    printf("i2c_slave_init: unmasking RD_REQ, RX_FULL, TX_ABRT\n");
    self.i2c->hw->intr_mask &= ~(I2C_IC_INTR_MASK_M_RD_REQ_BITS
                               | I2C_IC_INTR_MASK_M_RX_FULL_BITS
                               | I2C_IC_INTR_MASK_M_TX_ABRT_BITS);
    printf("i2c_slave_init: unmask done\n");
}
