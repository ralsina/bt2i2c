#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include <btstack.h>
#include <stdio.h>

#include "bt_keyboard.h"
#include "i2c_slave.h"
#include "pins.h"

static btstack_timer_source_t heartbeat;

static void heartbeat_handler(btstack_timer_source_t *ts)
{
    static int count = 0;
    printf("[%d] alive\n", count++);

    // Turn LED on only when keyboard is connected, off otherwise
    extern bool bt_keyboard_is_connected(void);
    bool connected = bt_keyboard_is_connected();
    cyw43_arch_gpio_put(PICO_W_LED, connected ? 1 : 0);

    // If we're in IDLE state (disconnected and waiting), restart scanning
    extern bool bt_keyboard_reconnect_if_needed(void);
    bt_keyboard_reconnect_if_needed();

    btstack_run_loop_set_timer(&heartbeat, 3000);
    btstack_run_loop_add_timer(&heartbeat);
}

static void pulse_led(void)
{
    cyw43_arch_gpio_put(PICO_W_LED, 1);
    busy_wait_ms(50);
    cyw43_arch_gpio_put(PICO_W_LED, 0);
}

int main(void)
{
    stdio_init_all();

    // wait for USB serial to enumerate before first print
    sleep_ms(2000);

    printf("BT2I2C Bridge v0.3\n");

    // Init I2C slave on GP4/SDA GP5/SCL
    i2c_slave_init();
    printf("I2C slave on GP%d(SDA)/GP%d(SCL) addr=0x%02X\n",
           PIN_I2C_SDA, PIN_I2C_SCL, 0x1F);

    if (cyw43_arch_init()) {
        printf("cyw43_arch_init FAILED\n");
        while (1) { tight_loop_contents(); }
    }
    printf("cyw43_arch_init OK\n");

    btstack_main(0, NULL);
    printf("btstack_main returned\n");

    heartbeat.process = &heartbeat_handler;
    btstack_run_loop_set_timer(&heartbeat, 3000);
    btstack_run_loop_add_timer(&heartbeat);

    // Make sure LED starts off
    cyw43_arch_gpio_put(PICO_W_LED, 0);

    btstack_run_loop_execute();

    return 0;
}
