#pragma once

#include "bt_keyboard.h"
#include <stdint.h>
#include <stdbool.h>

#define KEY_FIFO_SIZE 31

struct fifo_item
{
    uint8_t key;
    enum key_state state;
};

uint8_t fifo_count(void);
void fifo_flush(void);
bool fifo_enqueue(struct fifo_item item);
void fifo_enqueue_force(struct fifo_item item);
struct fifo_item fifo_dequeue(void);
