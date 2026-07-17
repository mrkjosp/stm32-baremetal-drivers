/* uart.h — bare-metal USART1 driver: polled TX, interrupt-driven RX + ring buffer */
#ifndef UART_H
#define UART_H

#include "stm32f405.h"
#include <stdbool.h>

/* Configure USART1 on PA9 (TX) / PA10 (RX) at the given baud, with RX interrupt. */
void uart_init(uint32_t baud);

/* Polled transmit: blocks until the byte is accepted by the hardware. */
void uart_send_byte(uint8_t b);
void uart_send_string(const char *s);

/* Non-blocking receive: pulls one byte from the RX ring buffer.
 * Returns false if no byte is available. */
bool uart_receive_byte(uint8_t *out);

#endif /* UART_H */