/* FellaPC Firmaware
 * VGA
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#ifndef VGA_H_
#define VGA_H_

#include <stdint.h>

void vga_newLine(void);

void vga_vsync(void);

void vga_scroll(void);

void vga_set(char c);

char vga_get(void);

uint8_t vga_getLine(char *line);

void vga_putc(char c);

void vga_puts(const char *str);

void vga_clear(void);

void vga_setCursor(uint8_t col, uint8_t row);

void vga_moveCursor(int8_t col, int8_t row);

void vga_tabCursor(void);

void vga_resetCursor(void);

void vga_handleCursor(void);

#endif
