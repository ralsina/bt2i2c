#pragma once

#include "pico.h"

// I2C pins to host (GP4 = SDA, GP5 = SCL) using I2C0
#define PIN_I2C_SDA         4
#define PIN_I2C_SCL         5
#define I2C_INST            i2c0

// Pico W on-board LED via CYW43
#define PICO_W_LED 0
