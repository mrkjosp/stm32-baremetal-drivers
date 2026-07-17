/* ============================================================================
 * ringbuffer.h — lock-free single-producer/single-consumer byte ring buffer

 * Used so the UART RX interrupt (producer) can hand bytes to the main loop
 * (consumer) without locks. It is lock-free and safe WITHOUT disabling
 * interrupts because:
 *   - only the ISR writes 'head'
 *   - only the main loop writes 'tail'
 * Each index has a single writer, so there's no race on either one. 'volatile'
 * stops the compiler caching them across the ISR/main boundary.
 * Capacity is a power of two so wrap-around is a cheap bitwise AND (& MASK)
 * instead of a modulo (%), which matters on an MCU with no fast divider.
 * ========================================================================== */
#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdint.h>
#include <stdbool.h>

#define RB_SIZE 128            /* must be a power of two */
#define RB_MASK (RB_SIZE - 1)

typedef struct {
    uint8_t  buf[RB_SIZE];
    volatile uint16_t head;    /* written by producer (ISR) only */
    volatile uint16_t tail;    /* written by consumer (main) only */
} ringbuffer_t;

void rb_init(ringbuffer_t *rb);
bool rb_push(ringbuffer_t *rb, uint8_t b);  /* producer; false if full */
bool rb_pop(ringbuffer_t *rb, uint8_t *out);/* consumer; false if empty */

#endif /* RINGBUFFER_H */