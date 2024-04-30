/* FellaPC Firmaware
 * TTY
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#include <stdint.h>
#include <string.h>

#include "keyboard.h"
#include "keys.h"
#include "vga.h"

#define AUTOREPEAT_THRESHOLD 50

static uint8_t caps = 0;
static char line[VGA_COLS + 1];

static char tty_key2ascii(uint8_t mod, uint8_t key)
{
	static const char lower[] = "abcdefghijklmnopqrstuvwxyz1234567890\0\0\0\t -=[]\\\0;'`,./";
	static const char upper[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()\0\0\0\t _+{}|\0:\"~<>?";

	char ret = '\0';
	uint8_t shift = !!(mod & (KEY_MOD_LSHIFT | KEY_MOD_RSHIFT));
	uint8_t letter = !!((key >= KEY_A) && (key <= KEY_Z));

	if (letter && caps) {
		shift = !shift;
	}

	if (key >= 4) {
		key -= 4;
		if (key <= sizeof(lower)) {
			ret = shift ? upper[key] : lower[key];
		}
	}

	return ret;
}

static void tty_set(char c)
{
	line[vga_getCol()] = c;
	vga_set(c);
}

static uint8_t tty_handleSpecial(uint8_t mod, uint8_t key)
{
	uint8_t start, row;
	(void)mod;

	switch (key) {
		case KEY_CAPSLOCK:
			caps = !caps;
			break;

		case KEY_HOME:
			vga_moveCursor(-80, 0);
			break;

		case KEY_DELETE:
			tty_set(' ');
			break;

		case KEY_END:
			while (line[vga_getCol()] != '\0' && vga_getCol() < VGA_COLS - 1) {
				vga_moveCursor(1, 0);
			}
			break;

		case KEY_RIGHT:
			if (line[vga_getCol()] != '\0') {
				vga_moveCursor(1, 0);
			}
			break;

		case KEY_LEFT:
			vga_moveCursor(-1, 0);
			break;

		case KEY_BACKSPACE:
			start = vga_getCol();
			if (start != 0) {
				row = vga_getRow();
				memmove(line + start - 1, line + start, sizeof(line) - start);
				vga_setCursor(0, row);
				vga_puts(line);
				vga_putc(' ');
				vga_setCursor(start - 1, row);
			}
			break;

		default:
			return 0;
	}

	return 1;
}

int8_t tty_update(char *cmd)
{
	static struct keys keys = { 0 };
	static uint8_t last_key = KEY_NONE;
	static uint8_t arcnt = 0;
	uint8_t key;

	if (keyboard_scan(&keys) < 0) {
		return -1;
	}

	/* Ignore multiple keys being pressed */
	key = keys.keys[0];

	if (key == last_key && key != KEY_NONE) {
		if (arcnt < AUTOREPEAT_THRESHOLD) {
			++arcnt;
		}
	}
	else {
		arcnt = 0;
	}

	if (key != last_key || arcnt >= AUTOREPEAT_THRESHOLD) {
		last_key = key;

		if (key == KEY_ENTER) {
			strcpy(cmd, line);
			memset(line, '\0', sizeof(line));
			vga_newLine();
			return 1;
		}

		if (tty_handleSpecial(keys.mod, key)) {
			return 0;
		}
		else {
			char c = tty_key2ascii(keys.mod, key);
			if (c != '\0') {
				tty_set(c);
				vga_moveCursor(1, 0);
				return 0;
			}
		}
	}

	vga_handleCursor();

	return 0;
}
