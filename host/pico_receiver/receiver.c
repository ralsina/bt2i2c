#include <pico/stdlib.h>
#include <hardware/uart.h>
#include <stdio.h>
#include <string.h>

#define UART_ID    uart1
#define PIN_RX     5
#define BAUD_RATE  115200

#define HID_REPORT_SIZE 8
#define SYNC_BYTE       0xFE

static const char *key_names[0x53] = {
    [0x00] = "None",
    [0x04] = "A", [0x05] = "B", [0x06] = "C", [0x07] = "D",
    [0x08] = "E", [0x09] = "F", [0x0A] = "G", [0x0B] = "H",
    [0x0C] = "I", [0x0D] = "J", [0x0E] = "K", [0x0F] = "L",
    [0x10] = "M", [0x11] = "N", [0x12] = "O", [0x13] = "P",
    [0x14] = "Q", [0x15] = "R", [0x16] = "S", [0x17] = "T",
    [0x18] = "U", [0x19] = "V", [0x1A] = "W", [0x1B] = "X",
    [0x1C] = "Y", [0x1D] = "Z",
    [0x1E] = "1", [0x1F] = "2", [0x20] = "3", [0x21] = "4",
    [0x22] = "5", [0x23] = "6", [0x24] = "7", [0x25] = "8",
    [0x26] = "9", [0x27] = "0",
    [0x28] = "Enter", [0x29] = "Escape", [0x2A] = "Backspace",
    [0x2B] = "Tab", [0x2C] = "Space",
    [0x2D] = "-", [0x2E] = "=", [0x2F] = "[", [0x30] = "]",
    [0x31] = "\\", [0x32] = "#", [0x33] = ";", [0x34] = "'",
    [0x35] = "`", [0x36] = ",", [0x37] = ".", [0x38] = "/",
    [0x39] = "CapsLock",
    [0x4C] = "Delete",
};

static const char *mod_names[8] = {
    "LCtrl", "LShift", "LAlt", "LGui",
    "RCtrl", "RShift", "RAlt", "RGui",
};

static uint8_t prev_report[HID_REPORT_SIZE];
static bool has_prev = false;

static bool read_frame(uint8_t *report)
{
    uint8_t sync;
    while (true) {
        int c = uart_getc(UART_ID);
        if (c == PICO_ERROR_TIMEOUT) continue;
        sync = (uint8_t)c;
        if (sync == SYNC_BYTE) break;
    }

    for (int i = 0; i < HID_REPORT_SIZE; i++) {
        int c = uart_getc(UART_ID);
        if (c == PICO_ERROR_TIMEOUT) {
            printf("TIMEOUT reading report byte %d\n", i);
            return false;
        }
        report[i] = (uint8_t)c;
    }
    return true;
}

static void print_key_event(const char *name, bool pressed)
{
    printf("KEY %s: %s\n", pressed ? "press" : "release", name);
}

static void process_report(const uint8_t *report)
{
    uint8_t mod = report[0];
    uint8_t prev_mod = has_prev ? prev_report[0] : 0;

    uint8_t mod_changes = mod ^ prev_mod;
    for (int m = 0; m < 8; m++) {
        if (mod_changes & (1 << m)) {
            print_key_event(mod_names[m], mod & (1 << m));
        }
    }

    for (int i = 2; i < HID_REPORT_SIZE; i++) {
        uint8_t code = report[i];
        if (code == 0 || code > 0x52) continue;

        bool in_prev = false;
        if (has_prev) {
            for (int j = 2; j < HID_REPORT_SIZE; j++) {
                if (prev_report[j] == code) { in_prev = true; break; }
            }
        }

        bool in_cur = false;
        for (int j = 2; j < HID_REPORT_SIZE; j++) {
            if (report[j] == code) { in_cur = true; break; }
        }

        const char *name = key_names[code] ? key_names[code] : "Unknown";
        if (in_cur && !in_prev)
            print_key_event(name, true);
        else if (!in_cur && in_prev)
            print_key_event(name, false);
    }

    memcpy(prev_report, report, HID_REPORT_SIZE);
    has_prev = true;
}

int main(void)
{
    stdio_init_all();
    sleep_ms(2000);
    printf("Pico Receiver starting...\n");

    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(PIN_RX, GPIO_FUNC_UART);
    uart_set_fifo_enabled(UART_ID, false);

    printf("Listening on UART1 GP%d at %d baud (sync=0x%02X)\n",
           PIN_RX, BAUD_RATE, SYNC_BYTE);

    uint8_t report[HID_REPORT_SIZE];
    while (true) {
        if (read_frame(report)) {
            printf("RAW: ");
            for (int i = 0; i < HID_REPORT_SIZE; i++)
                printf("%02x ", report[i]);
            printf("\n");
            process_report(report);
        }
        tight_loop_contents();
    }
}
