#include "fifo.h"
#include <hardware/sync.h>

static struct
{
    struct fifo_item fifo[KEY_FIFO_SIZE];
    uint8_t count;
    uint8_t read_idx;
    uint8_t write_idx;
} self;

static bool fifo_enqueue_unsafe(struct fifo_item item)
{
    if (self.count >= KEY_FIFO_SIZE)
        return false;

    self.fifo[self.write_idx++] = item;
    self.write_idx %= KEY_FIFO_SIZE;
    ++self.count;

    return true;
}

uint8_t fifo_count(void)
{
    uint32_t status = save_and_disable_interrupts();
    uint8_t count = self.count;
    restore_interrupts(status);
    return count;
}

void fifo_flush(void)
{
    uint32_t status = save_and_disable_interrupts();
    self.write_idx = 0;
    self.read_idx = 0;
    self.count = 0;
    restore_interrupts(status);
}

bool fifo_enqueue(struct fifo_item item)
{
    uint32_t status = save_and_disable_interrupts();
    bool success = fifo_enqueue_unsafe(item);
    restore_interrupts(status);
    return success;
}

void fifo_enqueue_force(struct fifo_item item)
{
    uint32_t status = save_and_disable_interrupts();
    if (fifo_enqueue_unsafe(item)) {
        restore_interrupts(status);
        return;
    }

    self.fifo[self.write_idx++] = item;
    self.write_idx %= KEY_FIFO_SIZE;
    ++self.read_idx;
    self.read_idx %= KEY_FIFO_SIZE;
    restore_interrupts(status);
}

struct fifo_item fifo_dequeue(void)
{
    uint32_t status = save_and_disable_interrupts();
    struct fifo_item item = {0};
    if (self.count == 0) {
        restore_interrupts(status);
        return item;
    }

    item = self.fifo[self.read_idx++];
    self.read_idx %= KEY_FIFO_SIZE;
    --self.count;

    restore_interrupts(status);
    return item;
}
