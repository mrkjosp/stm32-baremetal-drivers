/* ============================================================================
 * spi.c — bare-metal SPI1 master driver for STM32F405 (register-level, no HAL)
 *
 * Validation: logic-validated against the RM, not executed end-to-end in QEMU
 * (incomplete SPI model). See README. Pins (PA5/6/7 = SCK/MISO/MOSI, AF5) and
 * registers follow RM0090. Slave-select is handled in software here (SSM/SSI)
 * so the example doesn't depend on a specific external device's wiring.
 * ========================================================================== */
#include "spi.h"
#include "gpio.h"

void spi_init(spi_mode_t mode)
{
    /* 1. Clocks: GPIOA for the pins, SPI1 on APB2. */
    gpio_enable_port(GPIOA);
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    /* 2. PA5=SCK, PA6=MISO, PA7=MOSI as alternate function AF5 (SPI1). */
    gpio_set_mode(GPIOA, 5, GPIO_ALTERNATE);
    gpio_set_mode(GPIOA, 6, GPIO_ALTERNATE);
    gpio_set_mode(GPIOA, 7, GPIO_ALTERNATE);
    gpio_set_alternate(GPIOA, 5, 5);
    gpio_set_alternate(GPIOA, 6, 5);
    gpio_set_alternate(GPIOA, 7, 5);

    /* 3. Configure CR1:
     *    - set CPOL/CPHA from the requested mode (low 2 bits map directly)
     *    - MSTR = master
     *    - BR field = clock divider (here /16 = 0b011 << 3) — pick per device
     *    - SSM + SSI = manage slave-select in software, hold it high internally
     *    - SPE = enable last */
    uint32_t cr1 = 0;
    if (mode & 0x1) cr1 |= SPI_CR1_CPHA;
    if (mode & 0x2) cr1 |= SPI_CR1_CPOL;
    cr1 |= SPI_CR1_MSTR;
    cr1 |= (0x3U << 3);            /* baud = fPCLK/16 */
    cr1 |= SPI_CR1_SSM | SPI_CR1_SSI;
    SPI1->CR1 = cr1;
    SPI1->CR1 |= SPI_CR1_SPE;
}

uint8_t spi_transfer(uint8_t tx)
{
    /* Full-duplex: every clock that shifts a bit OUT also shifts a bit IN.
     * 1) wait until the TX buffer is empty, then write the byte to send
     * 2) wait until the RX buffer has a byte, then read it (the response) */
    while (!(SPI1->SR & SPI_SR_TXE)) { }
    SPI1->DR = tx;
    while (!(SPI1->SR & SPI_SR_RXNE)) { }
    return (uint8_t)(SPI1->DR & 0xFF);
}
