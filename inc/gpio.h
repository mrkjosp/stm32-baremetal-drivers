/* gpio.h — bare-metal GPIO driver (register-level, no HAL) */
#ifndef GPIO_H
#define GPIO_H

#include "stm32f405.h"

typedef enum { GPIO_INPUT = 0, GPIO_OUTPUT = 1, GPIO_ALTERNATE = 2, GPIO_ANALOG = 3 } gpio_mode_t;

/* Enable a GPIO port's clock  */
void gpio_enable_port(GPIO_TypeDef *port);

/* Set the mode (input/output/alternate/analog) of one pin (0-15). */
void gpio_set_mode(GPIO_TypeDef *port, uint8_t pin, gpio_mode_t mode);

/* For alternate-function pins, select which AF (0-15)  */
void gpio_set_alternate(GPIO_TypeDef *port, uint8_t pin, uint8_t af);

/* Drive an output pin high (1) or low (0). */
void gpio_write(GPIO_TypeDef *port, uint8_t pin, uint8_t value);

/* Read an input pin's level (returns 0 or 1). */
uint8_t gpio_read(GPIO_TypeDef *port, uint8_t pin);

#endif /* GPIO_H */
