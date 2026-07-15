/* gpio.c — bare-metal GPIO driver for STM32F405 (register-level, no HAL)
 *
 * Each pin's settings live as a FIELD inside a shared register. MODER, for
 * example, uses 2 bits per pin, so pin N's mode is at bit position (N*2).  */
#include "gpio.h"

void gpio_enable_port(GPIO_TypeDef *port)
{
    /* GPIO ports sit 0x400 apart on AHB1, and RCC_AHB1ENR's enable bits follow
     * the same order (bit 0 = GPIOA, bit 1 = GPIOB, ...), so the port's byte
     * offset from AHB1PERIPH_BASE directly gives its enable-bit position. */
    uint32_t idx = ((uint32_t)port - AHB1PERIPH_BASE) / 0x400UL;
    RCC->AHB1ENR |= (1U << idx);
}

void gpio_set_mode(GPIO_TypeDef *port, uint8_t pin, gpio_mode_t mode)
{
    /* MODER: 2 bits per pin. Clear the pin's 2-bit field, then set new mode. */
    port->MODER &= ~(0x3U << (pin * 2));
    port->MODER |=  ((uint32_t)mode << (pin * 2));
}

void gpio_set_alternate(GPIO_TypeDef *port, uint8_t pin, uint8_t af)
{
    /* AFR is two 32-bit registers, 4 bits per pin. Pins 0-7 in AFR[0],
     * pins 8-15 in AFR[1]. */
    uint8_t idx   = pin / 8;          /* which AFR register */
    uint8_t shift = (pin % 8) * 4;    /* bit offset within it */
    port->AFR[idx] &= ~(0xFU << shift);
    port->AFR[idx] |=  ((uint32_t)af << shift);
}

void gpio_write(GPIO_TypeDef *port, uint8_t pin, uint8_t value)
{
    /* BSRR is the atomic set/reset register: writing a 1 to the low 16 bits
     * SETS a pin, writing a 1 to the high 16 bits RESETS it. Using BSRR avoids
     * a read-modify-write race on ODR if an interrupt also touches the port. */
    if (value) port->BSRR = (1U << pin);          /* set   */
    else       port->BSRR = (1U << (pin + 16));   /* reset */
}

uint8_t gpio_read(GPIO_TypeDef *port, uint8_t pin)
{
    return (port->IDR >> pin) & 0x1U;
}