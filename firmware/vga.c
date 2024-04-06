/* FellaPC Firmaware
 * VGA
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#include "vga.h"

extern volatile uint8_t g_vsync;

static struct {
	volatile uint8_t data;
	volatile uint8_t col;
	volatile uint8_t row;
} * const vga = (void *)0x8000;

void vga_vsync(void)
{
	g_vsync = 0;
	while (!g_vsync);
}

static void vga_setAddr(uint8_t row, uint8_t col)
{
	vga->row = row;
	vga->col = col;
}

void vga_write(uint8_t row, uint8_t col, uint8_t data)
{
	vga_setAddr(row, col);
	vga->data = data;
}

uint8_t vga_read(uint8_t row, uint8_t col)
{
	vga_setAddr(row, col);
	return vga->data;
}

