/* spi.h — bare-metal SPI master driver (register-level, no HAL)
 *
 * NOTE: logic-validated, not executed end-to-end under QEMU (its SPI peripheral
 * model is incomplete). See README "How this was validated". The transaction
 * logic follows the STM32F4 Reference Manual SPI chapter and would run against
 * a simulated device in Renode or on real silicon. */
#ifndef SPI_H
#define SPI_H

#include "stm32f405.h"

/* SPI mode = (CPOL, CPHA). CPOL sets the idle clock level; CPHA sets which
 * clock edge samples data. Master and slave MUST agree or every byte is wrong. */
typedef enum {
    SPI_MODE0 = 0,  /* CPOL=0 CPHA=0 */
    SPI_MODE1 = 1,  /* CPOL=0 CPHA=1 */
    SPI_MODE2 = 2,  /* CPOL=1 CPHA=0 */
    SPI_MODE3 = 3,  /* CPOL=1 CPHA=1 */
} spi_mode_t;

void    spi_init(spi_mode_t mode);
uint8_t spi_transfer(uint8_t tx);   /* full-duplex: send one byte, return the byte received */

#endif /* SPI_H */
