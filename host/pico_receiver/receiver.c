#include <pico/stdlib.h>
#include <hardware/i2c.h>
#include <stdio.h>
#include <string.h>

#define I2C_INST     i2c0
#define PIN_SDA      4
#define PIN_SCL      5
#define I2C_ADDR     0x1F
#define I2C_SPEED    100000

#define REG_ID_VER   0x01
#define REG_ID_CFG   0x02
#define REG_ID_INT   0x03
#define REG_ID_KEY   0x04
#define REG_ID_FIF   0x09

#define KEY_COUNT_MASK 0x1F

enum key_state
{
    KEY_STATE_IDLE = 0,
    KEY_STATE_PRESSED,
    KEY_STATE_HOLD,
    KEY_STATE_RELEASED,
};

static const char *key_names[0x9E] = {
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
    [0x3A] = "F1", [0x3B] = "F2", [0x3C] = "F3", [0x3D] = "F4",
    [0x3E] = "F5", [0x3F] = "F6", [0x40] = "F7", [0x41] = "F8",
    [0x42] = "F9", [0x43] = "F10", [0x44] = "F11", [0x45] = "F12",
    [0x4C] = "Delete",
    [0x9A] = "Alt", [0x9B] = "LShift", [0x9C] = "RShift", [0x9D] = "Sym",
};

static const char *state_names[4] = {
    "IDLE", "PRESSED", "HOLD", "RELEASED"
};

static int reg_read(uint8_t reg, uint8_t *buf, uint8_t len)
{
    int ret = i2c_write_blocking(I2C_INST, I2C_ADDR, &reg, 1, true);
    if (ret != 1) return -1;
    return i2c_read_blocking(I2C_INST, I2C_ADDR, buf, len, false);
}

static int reg_read_byte(uint8_t reg)
{
    uint8_t val;
    if (reg_read(reg, &val, 1) != 1) return -1;
    return val;
}

int main(void)
{
    stdio_init_all();
    sleep_ms(2000);
    printf("Pico I2C Receiver starting...\n");

    i2c_init(I2C_INST, I2C_SPEED);
    gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_SDA);
    gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_SCL);

    sleep_ms(100);

    int ver = reg_read_byte(REG_ID_VER);
    if (ver < 0) {
        printf("No I2C device found at 0x%02X!\n", I2C_ADDR);
    } else {
        printf("BT2I2C firmware v%d.%d detected\n", ver >> 4, ver & 0x0F);
    }

    uint8_t fifo_buf[2];
    while (true) {
        int key_val = reg_read_byte(REG_ID_KEY);
        if (key_val < 0) {
            sleep_ms(10);
            continue;
        }

        uint8_t count = key_val & KEY_COUNT_MASK;
        if (count == 0) {
            sleep_ms(5);
            continue;
        }

        for (uint8_t i = 0; i < count; i++) {
            if (reg_read(REG_ID_FIF, fifo_buf, 2) != 2) {
                break;
            }

            uint8_t state = fifo_buf[0];
            uint8_t key = fifo_buf[1];
            const char *name = key_names[key] ? key_names[key] : "Unknown";
            const char *sname = state < 4 ? state_names[state] : "?";
            printf("KEY %s: %s (0x%02x)\n", sname, name, key);
        }

        tight_loop_contents();
    }
}
