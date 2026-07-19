/* i2c.h — bare-metal I2C master driver (register-level, no HAL)
 *
 * NOTE: logic-validated, not executed end-to-end under QEMU (incomplete I2C
 * model). Best run in Renode against a simulated sensor, or on real hardware
 * (which also needs external pull-up resistors on SDA/SCL). The start/address/
 * ACK/repeated-start sequence follows the STM32F4 Reference Manual I2C chapter. */
#ifndef I2C_H
#define I2C_H

#include "stm32f405.h"
#include <stdbool.h>

void i2c_init(void);

/* Read one register from a 7-bit-addressed device.
 * Returns true on success, with the value in *out. */
bool i2c_read_register(uint8_t dev_addr, uint8_t reg, uint8_t *out);

#endif /* I2C_H */
