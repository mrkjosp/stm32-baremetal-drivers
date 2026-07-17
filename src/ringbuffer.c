/* ringbuffer.c — see ringbuffer.h for the design rationale */
#include "ringbuffer.h"

void rb_init(ringbuffer_t *rb)
{
    rb->head = 0;
    rb->tail = 0;
}

bool rb_push(ringbuffer_t *rb, uint8_t b)
{
    uint16_t next = (rb->head + 1) & RB_MASK;
    if (next == rb->tail) {
        return false;          /* full: one slot kept empty to tell full from empty */
    }
    rb->buf[rb->head] = b;
    rb->head = next;           /* publish AFTER writing the byte */
    return true;
}

bool rb_pop(ringbuffer_t *rb, uint8_t *out)
{
    if (rb->tail == rb->head) {
        return false;          /* empty */
    }
    *out = rb->buf[rb->tail];
    rb->tail = (rb->tail + 1) & RB_MASK;
    return true;
}
