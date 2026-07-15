/* main.c — demonstrates the bare-metal drivers
 * - Configures an LED pin (PD12) and toggles it in the main loop (GPIO driver).
*/
#include "stm32f405.h"
#include "gpio.h"

static void delay(volatile uint32_t n) { while (n--) { __asm__("nop"); } }

int main(void)
{
    gpio_enable_port(GPIOD);
    gpio_set_mode(GPIOD, 12, GPIO_OUTPUT);

    while (1) {
        gpio_write(GPIOD, 12, 1);
        delay(200000);
        gpio_write(GPIOD, 12, 0);
        delay(200000);
    }
}