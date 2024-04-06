/* FellaPC Firmaware
 * VGA
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#ifndef VGA_H_
#define VGA_H_

#include <stdint.h>

void vga_vsync(void);

void vga_write(uint8_t row, uint8_t col, uint8_t data);

uint8_t vga_read(uint8_t row, uint8_t col);

#endif
