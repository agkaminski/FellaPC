/* FellaPC Firmaware
 * VGA
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#include "vga.h"
#include "via.h"

#define CURSOR (char)219

extern volatile uint8_t g_vsync;

extern void vga_clr(void);

uint8_t g_cursor_col;
uint8_t g_cursor_row;

static struct {
	volatile uint8_t data;
	volatile uint8_t col;
	volatile uint8_t row;
} * const vga = (void *)0x8000;

static struct {
	uint8_t counter;
	char prev;
	uint8_t state;
} cursor;

void vga_vsync(void)
{
	g_vsync = 0;
	while (!g_vsync);
}

void vga_resetCursor(void)
{
	cursor.counter = 0;
	if (cursor.state) {
		vga_set(cursor.prev);
	}
	cursor.state = 0;
}

uint8_t vga_putLine(const char *line)
{
	/* We strongly assume that line can fit within current column */
	uint8_t pos;

	vga_vsync();
	vga->row = g_cursor_row;
	for (pos = 0; line[pos] != '\0'; ++pos) {
		vga->col = g_cursor_col + pos;
		vga->data = line[pos];
	}

	return pos;
}

void vga_newLine(void)
{
	vga_resetCursor();
	g_cursor_col = 0;
	if (g_cursor_row >= VGA_ROWS - 1) {
		vga_scroll();
	}
	else {
		++g_cursor_row;
	}
}

static void vga_incCol(uint8_t count)
{
	g_cursor_col += count;
	if (g_cursor_col >= VGA_COLS) {
		vga_newLine();
	}
}

void vga_set(char c)
{
	vga_vsync();
	vga->row = g_cursor_row;
	vga->col = g_cursor_col;
	vga->data = c;
	cursor.prev = c;
}

char vga_get(void)
{
	if (cursor.state) {
		return cursor.prev;
	}
	else {
		vga_vsync();
		vga->row = g_cursor_row;
		vga->col = g_cursor_col;
		return vga->data;
	}
}

void vga_putc(char c)
{
	vga_resetCursor();

	switch (c) {
		case '\0':
			return;

		case '\n':
			vga_newLine();
			break;

		case '\r':
			g_cursor_col = 0;
			break;

		case '\t':
			vga_incCol(4 - (g_cursor_col & 0x3));
			break;

		default:
			vga_vsync();
			vga->row = g_cursor_row;
			vga->col = g_cursor_col;
			vga->data = c;
			vga_incCol(1);
			break;
	}
}

void vga_puts(const char *str)
{
	char buff[VGA_COLS + 1];
	uint8_t pos = 0;
	char special = '\0';

	vga_resetCursor();

	while (*str != '\0') {
		for (pos = 0; pos < VGA_COLS - g_cursor_col && *str != '\0'; ++pos) {
			if (*str == '\n' || *str == '\r' || *str =='\t') {
				special = *(str++);
				break;
			}

			buff[pos] = *(str++);
		}

		if (pos != 0) {
			buff[pos] = '\0';
			vga_incCol(vga_putLine(buff));
		}

		if (special != '\0') {
			vga_putc(special);
			special = '\0';
		}
	}
}

void vga_clear(void)
{
	vga_resetCursor();
	g_cursor_col = 0;
	g_cursor_row = 0;
	vga_clr();
}

void vga_handleCursor(void)
{
	++cursor.counter;
	if (cursor.counter == 32) {
		cursor.state = 0;
		cursor.prev = vga_get();
		vga_vsync();
		vga->row = g_cursor_row;
		vga->col = g_cursor_col;
		vga->data = CURSOR;
		cursor.state = 1;
	}
	else if (cursor.counter == 64) {
		vga_resetCursor();
	}
}

void vga_selectRom(uint8_t rom)
{
	via_setDirB(0x30, 0x30);
	via_setPortB((rom & 0x3) << 4, 0x30);
}
