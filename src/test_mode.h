#pragma once

#include <stdint.h>
#include <stdbool.h>

// Initialize test mode (generates ABCDE repeatedly)
void test_mode_init(void);

// Update test mode (call periodically)
void test_mode_update(void);

// Check if test mode is enabled
bool test_mode_is_enabled(void);

// Enable/disable test mode
void test_mode_set_enabled(bool enabled);
