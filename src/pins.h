#pragma once

#include "pico.h"

// UART pins to host (GP4 = TX, GP5 = RX)
#define PIN_UART_TX         4
#define PIN_UART_RX         5
#define UART_ID             uart1
#define UART_BAUD           115200

// Pico W on-board LED via CYW43
#define PICO_W_LED 0
