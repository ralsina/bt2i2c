#pragma once

#include <stdint.h>

// Initialize I2C slave on configured pins
void i2c_slave_init(void);

// Sync I2C slave address (call after changing REG_ID_ADR)
void i2c_slave_sync_address(void);
