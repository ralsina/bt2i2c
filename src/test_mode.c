#include "test_mode.h"
#include "fifo.h"

#include <pico/stdlib.h>
#include <stdio.h>

static struct
{
    bool enabled;
    uint8_t test_chars[5];
    uint8_t current_index;
    uint32_t last_press_time;
    uint32_t last_release_time;
    bool key_pressed;
} self;

// Test characters: a, b, c, d, e
static const uint8_t TEST_CHARS[] = {'a', 'b', 'c', 'd', 'e'};

void test_mode_init(void)
{
    self.enabled = false;
    self.current_index = 0;
    self.last_press_time = 0;
    self.last_release_time = 0;
    self.key_pressed = false;

    // Copy test characters
    for (int i = 0; i < 5; i++) {
        self.test_chars[i] = TEST_CHARS[i];
    }

    printf("Test mode initialized (chars: a,b,c,d,e)\n");
}

bool test_mode_is_enabled(void)
{
    return self.enabled;
}

void test_mode_set_enabled(bool enabled)
{
    self.enabled = enabled;
    if (enabled) {
        printf("Test mode ENABLED - generating ABCDE key events\n");
    } else {
        printf("Test mode DISABLED\n");
    }
}

void test_mode_update(void)
{
    if (!self.enabled) return;

    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    // Press key every 500ms if no key is currently pressed
    if (!self.key_pressed && (current_time - self.last_press_time >= 500)) {
        uint8_t key = self.test_chars[self.current_index];

        // Enqueue key press event
        struct fifo_item item = {
            .key = key,
            .state = KEY_STATE_PRESSED
        };
        fifo_enqueue(item);

        printf("[TEST] Key '%c' PRESSED\n", key);

        self.key_pressed = true;
        self.last_release_time = current_time;
    }

    // Release key after 200ms
    if (self.key_pressed && (current_time - self.last_release_time >= 200)) {
        uint8_t key = self.test_chars[self.current_index];

        // Enqueue key release event
        struct fifo_item item = {
            .key = key,
            .state = KEY_STATE_RELEASED
        };
        fifo_enqueue(item);

        printf("[TEST] Key '%c' RELEASED\n", key);

        self.key_pressed = false;
        self.last_press_time = current_time;

        // Move to next character
        self.current_index = (self.current_index + 1) % 5;
    }
}
