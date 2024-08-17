/* FellaPC Firmaware
 * I2C EEPROM
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>

void i2c_write(uint16_t address, const void *data, uint16_t len);

void i2c_read(uint16_t address, void *data, uint16_t len);

#endif
