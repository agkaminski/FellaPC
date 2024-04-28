/* FellaPC Firmaware
 * VGA
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#include "vga.h"

#define COLS 80
#define ROWS 60

extern volatile uint8_t g_vsync;

extern void vga_clr(void);

static struct {
	volatile uint8_t data;
	volatile uint8_t col;
	volatile uint8_t row;
} * const vga = (void *)0x8000;

static struct {
	uint8_t col;
	uint8_t row;
	uint8_t cursor_state;
} vga_context;

void vga_vsync(void)
{
	g_vsync = 0;
	while (!g_vsync);
}

void vga_write(uint8_t row, uint8_t col, uint8_t data)
{
	vga->row = row;
	vga->col = col;
	vga->data = data;
}

uint8_t vga_read(uint8_t row, uint8_t col)
{
	vga->row = row;
	vga->col = col;
	return vga->data;
}

static uint8_t vga_putLine(const char *line, uint8_t start)
{
	/* We strongly assume that line can fit within current column */
	uint8_t pos;

	vga_vsync();
	vga->row = vga_context.row;
	for (pos = 0; line[pos] != '\0'; ++pos) {
		vga->col = start + pos;
		vga->data = line[pos];
	}

	return pos;
}

static void vga_incCol(uint8_t count)
{
	vga_context.col += count;
	if (vga_context.col >= COLS) {
		vga_context.col = 0;
		if (vga_context.row >= ROWS - 1) {
			vga_scroll();
		}
		else {
			++vga_context.row;
		}
	}
}

void vga_puts(const char *str)
{
	char buff[COLS + 1];
	uint8_t i, pos = 0;
	uint8_t end = 0;

	while (1) {
		for (i = 0; i < COLS - vga_context.col; ++i, ++pos) {
			switch (str[pos]) {
				case '\0':
					end = 1;
					--i;
					break;

				case '\n':
					buff[i] = '\0';
					vga_putLine(buff, vga_context.col);
					vga_context.col = 0;
					if (vga_context.row >= ROWS - 1) {
						vga_scroll();
					}
					else {
						++vga_context.row;
					}
					i = 0;
					break;

				case '\r':
					buff[i] = '\0';
					vga_putLine(buff, vga_context.col);
					vga_context.col = 0;
					i = 0;
					break;

				default:
					buff[i] = str[pos];
					break;
			}

			if (end) {
				break;
			}
		}

		if (i != 0) {
			buff[i + 1] = '\0';
			vga_incCol(vga_putLine(buff, vga_context.col));
		}

		if (end) {
			return;
		}
	}
}

void vga_clear(void)
{
	vga_context.col = 0;
	vga_context.row = 0;
	vga_clr();
}