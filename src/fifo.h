#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "reg.h"

#define KEY_FIFO_SIZE 31

struct fifo_item
{
    char key;
    enum key_state state;
};

uint8_t fifo_count(void);
void fifo_flush(void);
bool fifo_enqueue(const struct fifo_item item);
void fifo_enqueue_force(const struct fifo_item item);
struct fifo_item fifo_dequeue(void);

void fifo_set_capslock(bool on);
void fifo_set_numlock(bool on);
bool fifo_get_capslock(void);
bool fifo_get_numlock(void);
