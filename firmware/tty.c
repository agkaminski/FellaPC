/* FellaPC Firmaware
 * TTY
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#include <stdint.h>

#include "keyboard.h"
#include "keys.h"
#include "vga.h"

static uint8_t caps = 0;

static char tty_key2ascii(uint8_t mod, uint8_t key)
{
	static const char lower[] = "abcdefghijklmnopqrstuvwxyz1234567890\0\0\0\t -=[]\\\0;'`,./";
	static const char upper[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()\0\0\0\t _+{}|\0:\"~<>?";

	char ret = '\0';
	int shift = !!(mod & (KEY_MOD_LSHIFT | KEY_MOD_RSHIFT));

	if ((key >= 4) && ((key - 4) <= sizeof(lower))) {
		ret = (shift || (caps && (key >= KEY_A && key <= KEY_Z))) ? upper[key - 4] : lower[key - 4];
	}

	return ret;
}


static int tty_handleSpecial(uint8_t mod, uint8_t key)
{
	(void)mod;

	switch (key) {
		case KEY_CAPSLOCK:
			caps = !caps;
			break;

		case KEY_HOME:
			vga_moveCursor(-80, 0);
			break;

		case KEY_DELETE:
			vga_moveCursor(-1, 0);
			vga_putc(' ');
			break;

		case KEY_END:
			vga_moveCursor(80, 0);
			break;

		case KEY_RIGHT:
			vga_moveCursor(1, 0);
			break;

		case KEY_LEFT:
			vga_moveCursor(-1, 0);
			break;

		case KEY_DOWN:
			vga_moveCursor(0, 1);
			break;

		case KEY_UP:
			vga_moveCursor(0, -1);
			break;

		case KEY_BACKSPACE:
			vga_set(' ');
			vga_moveCursor(-1, 0);
			vga_set(' ');
			break;

		default: return 0;
	}

	return 1;
}

int tty_update(char *cmd)
{
	static struct keys keys = { 0 };
	static uint8_t last_key = KEY_NONE;
	uint8_t key;
	uint8_t ret;

	if (keyboard_scan(&keys) < 0) {
		return -1;
	}

	/* Ignore multiple keys being pressed */
	key = keys.keys[0];
	if (key != last_key) {
		last_key = key;

		if (key == KEY_ENTER) {
			ret = vga_getLine(cmd);
			vga_newLine();
			return ret;
		}

		if (tty_handleSpecial(keys.mod, key)) {
			return 0;
		}
		else {
			char c = tty_key2ascii(keys.mod, key);
			if (c != '\0') {
				vga_putc(c);
				return 0;
			}
		}
	}

	vga_handleCursor();

	return 0;
}
