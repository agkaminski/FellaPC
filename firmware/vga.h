/* FellaPC Firmaware
 * VGA
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#ifndef VGA_H_
#define VGA_H_

#include <stdint.h>

#define VGA_COLS 80
#define VGA_ROWS 60

extern uint8_t g_cursor_col;
extern uint8_t g_cursor_row;

void vga_resetCursor(void);

void vga_newLine(void);

void vga_vsync(void);

void vga_scroll(void);

void vga_set(char c);

char vga_get(void);

uint8_t vga_putLine(const char *line);

void vga_putc(char c);

void vga_puts(const char *str);

void vga_clear(void);

void vga_tabCursor(void);

void vga_handleCursor(void);

void vga_selectRom(uint8_t rom);

#endif
