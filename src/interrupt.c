#include "interrupt.h"
#include "reg.h"
#include "pins.h"
#include <pico/stdlib.h>

void interrupt_pulse(void)
{
    gpio_put(PIN_INT, 0);
    busy_wait_ms(reg_get_value(REG_ID_IND));
    gpio_put(PIN_INT, 1);
}

void interrupt_init(void)
{
    gpio_init(PIN_INT);
    gpio_set_dir(PIN_INT, GPIO_OUT);
    gpio_pull_up(PIN_INT);
    gpio_put(PIN_INT, true);
}
