/* FellaPC Firmaware
 * VGA
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#ifndef VGA_H_
#define VGA_H_

#include <stdint.h>

void vga_write(uint8_t row, uint8_t col, uint8_t data);

#endif
