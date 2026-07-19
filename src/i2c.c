/* ============================================================================
 * i2c.c — bare-metal I2C1 master driver for STM32F405 (register-level, no HAL)
 *
 * Validation: logic-validated against the RM, not executed end-to-end in QEMU.
 * Demonstrates the canonical "read a register from an I2C sensor" sequence,
 * which is the single most common I2C operation in embedded work:
 *
 *   START -> send (addr|W) -> send register index
 *   repeated START -> send (addr|R) -> read 1 byte with NACK -> STOP
 *
 * A repeated start (instead of a stop between the write and read) keeps
 * "set the register pointer, then read it" atomic against other bus masters;
 * a plain stop+start would usually work too, but another master could grab
 * the bus in the gap and move the device's internal pointer first.
 *
 * Pins: PB6=SCL, PB7=SDA, AF4. Real hardware also needs external pull-ups.
 * ========================================================================== */
#include "i2c.h"
#include "gpio.h"

/* Spin until a status-register-1 flag is set. Returns false on a crude timeout
 * so a missing/!ACKing device can't hang the whole system forever. */
static bool wait_sr1(uint32_t flag)
{
    uint32_t timeout = 100000;
    while (!(I2C1->SR1 & flag)) {
        if (--timeout == 0) return false;
    }
    return true;
}

void i2c_init(void)
{
    /* Enable GPIOB and I2C1 clocks. gpio_enable_port derives the AHB1ENR bit
     * from the port's address, so this works for GPIOB the same as any port. */
    gpio_enable_port(GPIOB);
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    /* PB6/PB7 to alternate function AF4, open-drain (required for I2C). */
    GPIOB->MODER  &= ~((0x3U << (6*2)) | (0x3U << (7*2)));
    GPIOB->MODER  |=  ((0x2U << (6*2)) | (0x2U << (7*2)));   /* alternate */
    GPIOB->OTYPER |=  ((1U << 6) | (1U << 7));               /* open-drain */
    GPIOB->AFR[0] &= ~((0xFU << (6*4)) | (0xFU << (7*4)));
    GPIOB->AFR[0] |=  ((0x4U << (6*4)) | (0x4U << (7*4)));   /* AF4 */

    /* Minimal timing setup for standard-mode 100 kHz off a 16 MHz APB1.
     * CR2 carries the peripheral clock in MHz; CCR/TRISE set the bit timing
     * (values per the RM standard-mode formulas). */
    I2C1->CR1 = 0;                 /* ensure disabled while configuring */
    I2C1->CR2 = 16;                /* APB1 clock = 16 MHz */
    I2C1->CCR = 80;                /* 100 kHz standard mode */
    I2C1->TRISE = 17;              /* max rise time + 1 */
    I2C1->CR1 |= I2C_CR1_PE;       /* enable peripheral */
}

bool i2c_read_register(uint8_t dev_addr, uint8_t reg, uint8_t *out)
{
    /* --- Phase 1: START + write the register index we want to read --- */
    I2C1->CR1 |= I2C_CR1_START;
    if (!wait_sr1(I2C_SR1_SB)) return false;        /* start condition sent */

    I2C1->DR = (uint8_t)(dev_addr << 1);            /* address + WRITE bit (0) */
    if (!wait_sr1(I2C_SR1_ADDR)) return false;      /* address acked */
    (void)I2C1->SR2;                                /* clearing ADDR: read SR1 then SR2 */

    if (!wait_sr1(I2C_SR1_TXE)) return false;
    I2C1->DR = reg;                                 /* send register index */
    if (!wait_sr1(I2C_SR1_BTF)) return false;       /* byte transferred */

    /* --- Phase 2: repeated START + read one byte --- */
    I2C1->CR1 |= I2C_CR1_START;
    if (!wait_sr1(I2C_SR1_SB)) return false;        /* repeated start */

    I2C1->DR = (uint8_t)((dev_addr << 1) | 1U);     /* address + READ bit (1) */
    if (!wait_sr1(I2C_SR1_ADDR)) return false;

    /* Single-byte read: disable ACK BEFORE clearing ADDR, so the master NACKs
     * the (only) byte, then program STOP. This ordering is required by the RM
     * for the 1-byte case; multi-byte reads would need ACK held until just
     * before the last byte instead. */
    I2C1->CR1 &= ~I2C_CR1_ACK;
    (void)I2C1->SR2;                                /* clear ADDR */
    I2C1->CR1 |= I2C_CR1_STOP;                      /* schedule stop after the byte */

    if (!wait_sr1(I2C_SR1_RXNE)) return false;
    *out = (uint8_t)(I2C1->DR & 0xFF);
    return true;
}