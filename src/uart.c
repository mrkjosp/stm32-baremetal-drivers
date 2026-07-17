/* ============================================================================
 * uart.c — bare-metal USART1 driver for STM32F405 (register-level, no HAL)
 *
 * Implements the full interrupt-driven receive path:
 *
 *     byte arrives on wire
 *        -> USART hardware sets RXNE flag, fires the USART1 interrupt
 *        -> USART1_IRQHandler reads DR, pushes byte into ring buffer  (PRODUCER)
 *        -> main loop calls uart_receive_byte() -> rb_pop()           (CONSUMER)
 *
 * The ISR is kept tiny: read the byte, push it, return. No processing in the
 * ISR 
 * NOTE ON PIN/PORT CHOICE: USART1 with TX=PA9, RX=PA10 is used because the
 * QEMU 'netduinoplus2' machine routes its serial console to USART1. On a real
 * enable bit (APB2 vs APB1), the pins, and the IRQ number change.
========================================================================== */
#include "uart.h"
#include "gpio.h"
#include "ringbuffer.h"

/* The RX ring buffer, shared between the ISR (producer) and main (consumer). */
static ringbuffer_t rx_rb;

void uart_init(uint32_t baud)
{
    /* 1. Enable clocks: GPIOA (for the pins) and USART1 (on APB2). */
    gpio_enable_port(GPIOA);
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    /* 2. Configure PA9 (TX) and PA10 (RX) as alternate-function, AF7 = USART1. */
    gpio_set_mode(GPIOA, 9,  GPIO_ALTERNATE);
    gpio_set_mode(GPIOA, 10, GPIO_ALTERNATE);
    gpio_set_alternate(GPIOA, 9,  7);   /* PA9  -> USART1_TX */
    gpio_set_alternate(GPIOA, 10, 7);   /* PA10 -> USART1_RX */

    /* 3. Baud rate. BRR = peripheral_clock / baud (oversampling-16 integer form).*/
    uint32_t pclk = 16000000U;
    USART1->BRR = pclk / baud;

    /* 4. Enable RX interrupt, transmitter, receiver, then the USART itself. */
    USART1->CR1 |= USART_CR1_RXNEIE;
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE;
    USART1->CR1 |= USART_CR1_UE;

    /* 5. Init the ring buffer before arming the interrupt, so there's no
     *    window where a byte could arrive before it's ready to receive it. */
    rb_init(&rx_rb);
    nvic_enable_irq(USART1_IRQn);
}

void uart_send_byte(uint8_t b)
{
    /* Polled TX: wait until transmit data register empty (TXE=1), then write. */
    while (!(USART1->SR & USART_SR_TXE)) { }
    USART1->DR = b;
}

void uart_send_string(const char *s)
{
    while (*s) {
        uart_send_byte((uint8_t)*s++);
    }
}

bool uart_receive_byte(uint8_t *out)
{
    return rb_pop(&rx_rb, out);
}

/* ---- The interrupt handler ----
 * Fires whenever a byte arrives (RXNE set). */
void USART1_IRQHandler(void)
{
    if (USART1->SR & USART_SR_RXNE) {
        uint8_t b = (uint8_t)(USART1->DR & 0xFF);  /* reading DR clears RXNE */
        rb_push(&rx_rb, b);                        /* drop silently if full */
    }
}