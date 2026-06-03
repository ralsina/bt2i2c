#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include <btstack.h>
#include <stdio.h>

#include "bt_keyboard.h"
#include "display.h"
#include "i2c_slave.h"
#include "reg.h"
#include "pins.h"
#include "fifo.h"

static btstack_timer_source_t heartbeat;
static bool pairing_button_pressed = false;

static void heartbeat_handler(btstack_timer_source_t *ts)
{
    static int count = 0;
    char status_msg[64];

    snprintf(status_msg, sizeof(status_msg),
             "[%d] %s - FIFO: %d",
             count++,
             bt_keyboard_is_connected() ? "CONNECTED" : "SEARCHING");

    printf("%s - BLE: %s - %d keys\n",
           status_msg,
           bt_keyboard_is_connected() ? "CONN" : "SEARCH",
           fifo_count());

    display_log(status_msg);

    // Turn LED on only when keyboard is connected, off otherwise
    bool connected = bt_keyboard_is_connected();
    cyw43_arch_gpio_put(PICO_W_LED, connected ? 1 : 0);

    // If we're in IDLE state (disconnected and waiting), restart scanning
    bt_keyboard_reconnect_if_needed();

    // Heartbeat log to confirm firmware is alive
    printf("[HEARTBEAT] Firmware is alive - tick %d\n", count);

    btstack_run_loop_set_timer(&heartbeat, 3000);
    btstack_run_loop_add_timer(&heartbeat);
}

// GPIO interrupt handler for pairing button
static void pairing_button_irq(uint gpio, uint32_t events)
{
    if (events & GPIO_IRQ_EDGE_FALL) {
        // Button pressed (pull-up, so falling edge = press)
        if (gpio == PIN_PAIRING_BUTTON) {
            pairing_button_pressed = true;
        }
    }
}

static void check_pairing_button(btstack_timer_source_t *ts)
{
    if (pairing_button_pressed) {
        pairing_button_pressed = false;

        // Debounce: check if button is still low
        if (!gpio_get(PIN_PAIRING_BUTTON)) {
            printf("\n🔘 PAIRING BUTTON PRESSED!\n");
            display_log("PAIRING button pressed");
            bt_keyboard_start_pairing();
        }
    }

    // Check again in 100ms
    btstack_run_loop_set_timer(ts, 100);
    btstack_run_loop_add_timer(ts);
}

int main(void)
{
    stdio_init_all();

    // Wait for USB serial to enumerate before first print
    sleep_ms(2000);

    printf("═══════════════════════════════════════════════════════\n");
    printf("   BT2I2C Bridge - BLE to I2C Keyboard\n");
    printf("═══════════════════════════════════════════════════════\n");

    // Initialize display first so we can show messages
    display_init();
    display_log("BT2I2C Bridge init...");

    // Init registers and I2C slave on GP4/SDA GP5/SCL
    reg_init();
    i2c_slave_init();
    const uint8_t i2c_addr = reg_get_value(REG_ID_ADR);
    printf("✅ I2C slave initialized: GP%d(SDA)/GP%d(SCL), addr=0x%02X\n",
           PIN_I2C_SDA, PIN_I2C_SCL, i2c_addr);
    char i2c_msg[64];
    snprintf(i2c_msg, sizeof(i2c_msg), "I2C: 0x%02X on GP%d/GP%d",
             i2c_addr, PIN_I2C_SDA, PIN_I2C_SCL);
    display_log(i2c_msg);

    // Initialize pairing button
    gpio_init(PIN_PAIRING_BUTTON);
    gpio_set_dir(PIN_PAIRING_BUTTON, GPIO_IN);
    gpio_pull_up(PIN_PAIRING_BUTTON);
    gpio_set_irq_enabled_with_callback(PIN_PAIRING_BUTTON,
                                       GPIO_IRQ_EDGE_FALL,
                                       true,
                                       pairing_button_irq);
    printf("✅ Pairing button initialized (GP%d)\n", PIN_PAIRING_BUTTON);
    display_log("Pairing button ready");

    if (cyw43_arch_init()) {
        printf("❌ cyw43_arch_init FAILED\n");
        display_log("ERROR: WiFi/BLE failed");
        while (1) { tight_loop_contents(); }
    }
    printf("✅ WiFi/BLE chip initialized\n");
    display_log("WiFi/BLE initialized");

    bt_keyboard_init();
    btstack_main(0, NULL);
    printf("✅ BLE stack initialized\n");
    display_log("BLE stack ready");

    // Set up heartbeat timer (3 seconds)
    heartbeat.process = &heartbeat_handler;
    btstack_run_loop_set_timer(&heartbeat, 3000);
    btstack_run_loop_add_timer(&heartbeat);

    // Set up button check timer (100ms)
    static btstack_timer_source_t pairing_timer;
    pairing_timer.process = &check_pairing_button;
    btstack_run_loop_set_timer(&pairing_timer, 100);
    btstack_run_loop_add_timer(&pairing_timer);

    // Make sure LED starts off
    cyw43_arch_gpio_put(PICO_W_LED, 0);

    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("   Searching for BLE keyboard...\n");
    printf("   Put your keyboard in pairing mode\n");
    printf("   LED will light when connected\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("🔘 Press button on GP%d to force pairing\n", PIN_PAIRING_BUTTON);
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");

    display_log("Searching for keyboard...");
    display_log("Press button to pair");

    // Run the main event loop
    btstack_run_loop_execute();

    return 0;
}
