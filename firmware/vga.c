/* FellaPC Firmaware
 * VGA
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#include "vga.h"
#include "tty.h"

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
} vga_context;

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

void vga_newLine(void)
{
	vga_resetCursor();
	vga_context.col = 0;
	if (vga_context.row >= ROWS - 1) {
		vga_scroll();
	}
	else {
		++vga_context.row;
	}
}

static void vga_incCol(uint8_t count)
{
	vga_context.col += count;
	if (vga_context.col >= COLS) {
		vga_newLine();
	}
}

void vga_set(char c)
{
	vga_vsync();
	vga->row = vga_context.row;
	vga->col = vga_context.col;
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
		vga->row = vga_context.row;
		vga->col = vga_context.col;
		return vga->data;
	}
}

uint8_t vga_getLine(char *line)
{
	uint8_t i;

	vga_resetCursor();

	vga_vsync();
	vga->row = vga_context.row;
	for (i = 0; i < vga_context.col; ++i) {
		vga->col = i;
		line[i] = vga->data;
	}
	line[i] = '\0';
	return i;
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
			vga_context.col = 0;
			break;

		case '\t':
			vga_incCol(4 - (vga_context.col & 0x3));
			break;

		default:
			vga_vsync();
			vga->row = vga_context.row;
			vga->col = vga_context.col;
			vga->data = c;
			vga_incCol(1);
			break;
	}
}

void vga_puts(const char *str)
{
	char buff[COLS + 1];
	uint8_t cols = 0, pos = 0;
	uint8_t end = 0;

	vga_resetCursor();

	while (!end) {
		while ((cols < COLS - vga_context.col) && !end) {
			switch (str[pos]) {
				case '\0':
					end = 1;
					break;

				case '\n':
					if (cols != 0) {
						buff[cols] = '\0';
						vga_putLine(buff, vga_context.col);
						cols = 0;
					}
					vga_newLine();
					break;

				case '\r':
					if (cols != 0) {
						buff[cols] = '\0';
						vga_putLine(buff, vga_context.col);
						cols = 0;
					}
					vga_context.col = 0;
					break;

				case '\t':
					if (cols != 0) {
						buff[cols] = '\0';
						vga_incCol(vga_putLine(buff, vga_context.col));
						cols = 0;
					}
					vga_incCol(4 - (vga_context.col & 0x3));
					break;

				default:
					buff[cols++] = str[pos];
					break;
			}

			++pos;
		}

		if (cols != 0) {
			buff[cols] = '\0';
			vga_incCol(vga_putLine(buff, vga_context.col));
		}
	}
}

void vga_clear(void)
{
	vga_resetCursor();
	vga_context.col = 0;
	vga_context.row = 0;
	vga_clr();
}

void vga_setCursor(uint8_t col, uint8_t row)
{
	vga_resetCursor();
	vga_context.col = col;
	vga_context.row = row;
}

void vga_moveCursor(int8_t col, int8_t row)
{
	int8_t tcol = vga_context.col + col;
	int8_t trow = vga_context.row + row;

	if (tcol < 0) {
		tcol = 0;
	}
	else if (tcol >= COLS) {
		tcol = COLS -1;
	}

	if (trow < 0) {
		trow = 0;
	}
	else if (trow >= ROWS) {
		trow = ROWS - 1;
	}

	vga_resetCursor();

	vga_context.col = tcol;
	vga_context.row = trow;
}

void vga_handleCursor(void)
{
	++cursor.counter;
	if (cursor.counter == 32) {
		cursor.state = 0;
		cursor.prev = vga_get();
		vga_vsync();
		vga->row = vga_context.row;
		vga->col = vga_context.col;
		vga->data = '_';
		cursor.state = 1;
	}
	else if (cursor.counter == 64) {
		vga_resetCursor();
	}
}
