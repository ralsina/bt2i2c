#pragma once

#include "pico.h"

// I2C pins for the i2c_puppet protocol slave
#define PIN_PUPPET_SDA    0
#define PIN_PUPPET_SCL    1

// Interrupt pin to the host (pulled low on key event)
#define PIN_INT           2

// Pico W on-board LED via CYW43
#define PICO_W_LED        0
