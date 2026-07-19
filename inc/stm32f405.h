/* stm32f405.h  —  Minimal register definitions (STM32F405RG)
 *
 * Peripherals are just memory at fixed addresses ("memory-mapped I/O"). Reading
 * or writing these addresses reads/writes hardware registers. Every address and
 * bit here comes from the STM32F4 Reference Manual (RM0090) and F405 datasheet.
 *
 * We model each peripheral as a struct of volatile uint32_t laid out to match
 * the register map, then place that struct at the peripheral's base address.
 */
#ifndef STM32F405_H
#define STM32F405_H

#include <stdint.h>

/*  Peripheral base addresses (from the memory map in the RM)  */
#define PERIPH_BASE      0x40000000UL
#define AHB1PERIPH_BASE  (PERIPH_BASE + 0x00020000UL)
#define APB1PERIPH_BASE  (PERIPH_BASE + 0x00000000UL)

#define RCC_BASE         (AHB1PERIPH_BASE + 0x3800UL)
#define GPIOA_BASE       (AHB1PERIPH_BASE + 0x0000UL)
#define GPIOB_BASE       (AHB1PERIPH_BASE + 0x0400UL)
#define GPIOD_BASE       (AHB1PERIPH_BASE + 0x0C00UL)
#define USART2_BASE      (APB1PERIPH_BASE + 0x4400UL)
#define USART1_BASE      (PERIPH_BASE + 0x00011000UL)  /* USART1 on APB2 */

/* ---- RCC: Reset and Clock Control ----
 * Controls which peripherals get a clock. A peripheral with no clock is dead —
 * it ignores all register writes.*/
typedef struct {
    volatile uint32_t CR;        /* 0x00 clock control */
    volatile uint32_t PLLCFGR;   /* 0x04 */
    volatile uint32_t CFGR;      /* 0x08 */
    volatile uint32_t CIR;       /* 0x0C */
    volatile uint32_t AHB1RSTR;  /* 0x10 */
    volatile uint32_t AHB2RSTR;  /* 0x14 */
    volatile uint32_t AHB3RSTR;  /* 0x18 */
    uint32_t          _r0;       /* 0x1C reserved */
    volatile uint32_t APB1RSTR;  /* 0x20 */
    volatile uint32_t APB2RSTR;  /* 0x24 */
    uint32_t          _r1[2];    /* 0x28-0x2C reserved */
    volatile uint32_t AHB1ENR;   /* 0x30 AHB1 peripheral clock enable */
    volatile uint32_t AHB2ENR;   /* 0x34 */
    volatile uint32_t AHB3ENR;   /* 0x38 */
    uint32_t          _r2;       /* 0x3C reserved */
    volatile uint32_t APB1ENR;   /* 0x40 APB1 peripheral clock enable */
    volatile uint32_t APB2ENR;   /* 0x44 */
} RCC_TypeDef;
#define RCC ((RCC_TypeDef *) RCC_BASE)

/* AHB1ENR bit positions: bit 0 = GPIOA, bit 3 = GPIOD */
#define RCC_AHB1ENR_GPIOAEN  (1U << 0)
#define RCC_AHB1ENR_GPIODEN  (1U << 3)
/* APB1ENR bit 17 = USART2 */
#define RCC_APB1ENR_USART2EN (1U << 17)
/* APB2ENR bit 4 = USART1 */
#define RCC_APB2ENR_USART1EN (1U << 4)

/* ---- GPIO: General Purpose I/O ---- */
typedef struct {
    volatile uint32_t MODER;   /* 0x00 mode: 2 bits per pin (in/out/alt/analog) */
    volatile uint32_t OTYPER;  /* 0x04 output type: push-pull / open-drain */
    volatile uint32_t OSPEEDR; /* 0x08 output speed */
    volatile uint32_t PUPDR;   /* 0x0C pull-up/pull-down */
    volatile uint32_t IDR;     /* 0x10 input data (read pin levels) */
    volatile uint32_t ODR;     /* 0x14 output data (write pin levels) */
    volatile uint32_t BSRR;    /* 0x18 bit set/reset (atomic) */
    volatile uint32_t LCKR;    /* 0x1C */
    volatile uint32_t AFR[2];  /* 0x20-0x24 alternate function low/high */
} GPIO_TypeDef;
#define GPIOA ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOD ((GPIO_TypeDef *) GPIOD_BASE)

/* ---- USART: serial port ---- */
typedef struct {
    volatile uint32_t SR;   /* 0x00 status */
    volatile uint32_t DR;   /* 0x04 data */
    volatile uint32_t BRR;  /* 0x08 baud rate */
    volatile uint32_t CR1;  /* 0x0C control 1 */
    volatile uint32_t CR2;  /* 0x10 control 2 */
    volatile uint32_t CR3;  /* 0x14 control 3 */
    volatile uint32_t GTPR; /* 0x18 */
} USART_TypeDef;
#define USART2 ((USART_TypeDef *) USART2_BASE)
#define USART1 ((USART_TypeDef *) USART1_BASE)

/* ---- SPI ---- */
#define SPI1_BASE (PERIPH_BASE + 0x00013000UL)  /* SPI1 on APB2 */
typedef struct {
    volatile uint32_t CR1;     /* 0x00 control 1 (CPOL,CPHA,MSTR,BR,SPE...) */
    volatile uint32_t CR2;     /* 0x04 control 2 */
    volatile uint32_t SR;      /* 0x08 status (TXE,RXNE,BSY) */
    volatile uint32_t DR;      /* 0x0C data */
    volatile uint32_t CRCPR;   /* 0x10 */
    volatile uint32_t RXCRCR;  /* 0x14 */
    volatile uint32_t TXCRCR;  /* 0x18 */
    volatile uint32_t I2SCFGR; /* 0x1C */
    volatile uint32_t I2SPR;   /* 0x20 */
} SPI_TypeDef;
#define SPI1 ((SPI_TypeDef *) SPI1_BASE)
#define RCC_APB2ENR_SPI1EN (1U << 12)
#define SPI_CR1_CPHA  (1U << 0)
#define SPI_CR1_CPOL  (1U << 1)
#define SPI_CR1_MSTR  (1U << 2)
#define SPI_CR1_BR    (0x7U << 3)  /* baud rate divider field */
#define SPI_CR1_SPE   (1U << 6)    /* SPI enable */
#define SPI_CR1_SSI   (1U << 8)    /* internal slave select */
#define SPI_CR1_SSM   (1U << 9)    /* software slave management */
#define SPI_SR_RXNE   (1U << 0)
#define SPI_SR_TXE    (1U << 1)
#define SPI_SR_BSY    (1U << 7)

/* ---- I2C ---- */
#define I2C1_BASE (APB1PERIPH_BASE + 0x5400UL)
typedef struct {
    volatile uint32_t CR1;    /* 0x00 */
    volatile uint32_t CR2;    /* 0x04 */
    volatile uint32_t OAR1;   /* 0x08 */
    volatile uint32_t OAR2;   /* 0x0C */
    volatile uint32_t DR;     /* 0x10 */
    volatile uint32_t SR1;    /* 0x14 */
    volatile uint32_t SR2;    /* 0x18 */
    volatile uint32_t CCR;    /* 0x1C */
    volatile uint32_t TRISE;  /* 0x20 */
    volatile uint32_t FLTR;   /* 0x24 */
} I2C_TypeDef;
#define I2C1 ((I2C_TypeDef *) I2C1_BASE)
#define RCC_APB1ENR_I2C1EN (1U << 21)
#define I2C_CR1_PE     (1U << 0)
#define I2C_CR1_START  (1U << 8)
#define I2C_CR1_STOP   (1U << 9)
#define I2C_CR1_ACK    (1U << 10)
#define I2C_SR1_SB     (1U << 0)   /* start bit sent */
#define I2C_SR1_ADDR   (1U << 1)   /* address sent/matched */
#define I2C_SR1_BTF    (1U << 2)   /* byte transfer finished */
#define I2C_SR1_RXNE   (1U << 6)
#define I2C_SR1_TXE    (1U << 7)

/* USART status register (SR) flags */
#define USART_SR_TXE   (1U << 7)  /* transmit data register empty */
#define USART_SR_TC    (1U << 6)  /* transmission complete */
#define USART_SR_RXNE  (1U << 5)  /* read data register not empty */
/* USART control register 1 (CR1) bits */
#define USART_CR1_UE     (1U << 13) /* USART enable */
#define USART_CR1_TE     (1U << 3)  /* transmitter enable */
#define USART_CR1_RE     (1U << 2)  /* receiver enable */
#define USART_CR1_RXNEIE (1U << 5)  /* RXNE interrupt enable */

/* ---- NVIC (interrupt controller) ----
 * The NVIC has several 32-bit "Interrupt Set-Enable Registers" (ISER0, ISER1,
 * ...). Each enables 32 IRQs. To enable IRQ n: set bit (n % 32) in ISER[n/32].
 * USART1 is IRQ 37, so it lives in ISER1 (37/32 = 1) at bit 5 (37 % 32 = 5). */
#define NVIC_ISER ((volatile uint32_t *)0xE000E100UL)  /* ISER[0], ISER[1], ... */
#define USART2_IRQn 38
#define USART1_IRQn 37

static inline void nvic_enable_irq(uint32_t irq)
{
    NVIC_ISER[irq >> 5] = (1U << (irq & 0x1F));   /* irq/32 and irq%32 */
}

#endif /* STM32F405_H */