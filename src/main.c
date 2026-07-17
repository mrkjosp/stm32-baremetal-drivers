/* main.c — demonstrates the bare-metal drivers
 *
 * - Configures an LED pin (PD12) and toggles it in the main loop (GPIO driver).
 * - Initializes USART1 and echoes back every character received, via the
 *   interrupt-driven RX path + ring buffer (UART driver).
 *
 * Run in QEMU: characters typed in the serial console are echoed back,
 * proving the ISR -> ring buffer -> main loop path works without dropping bytes. */
#include "stm32f405.h"
#include "gpio.h"
#include "uart.h"

static void delay(volatile uint32_t n) { while (n--) { __asm__("nop"); } }

int main(void)
{
    /* --- GPIO: set up an LED on PD12 --- */
    gpio_enable_port(GPIOD);
    gpio_set_mode(GPIOD, 12, GPIO_OUTPUT);

    /* --- UART: 115200 baud, interrupt-driven RX --- */
    uart_init(115200);
    uart_send_string("\r\nSTM32 bare-metal drivers online.\r\n");
    uart_send_string("Type characters.\r\n");

    uint8_t c;
    while (1) {
        /* Echo any received bytes (consumer side of the ring buffer). */
        if (uart_receive_byte(&c)) {
            uart_send_byte(c);
        }

        /* Blink the LED so there's a visible heartbeat. */
        gpio_write(GPIOD, 12, 1);
        delay(200000);
        gpio_write(GPIOD, 12, 0);
        delay(200000);
    }
}