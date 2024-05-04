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
	line[g_cursor_col] = c;
	vga_set(c);
}

static uint8_t tty_handleSpecial(uint8_t mod, uint8_t key)
{
	uint8_t start;
	(void)mod;

	switch (key) {
		case KEY_CAPSLOCK:
			caps = !caps;
			break;

		case KEY_HOME:
			g_cursor_col = 0;
			break;

		case KEY_DELETE:
			tty_set(' ');
			break;

		case KEY_END:
			while (line[g_cursor_col] != '\0' && g_cursor_col < VGA_COLS - 1) {
				++g_cursor_col;
			}
			break;

		case KEY_RIGHT:
			if (line[g_cursor_col] != '\0') {
				++g_cursor_col;
			}
			break;

		case KEY_LEFT:
			if (g_cursor_col > 0) {
				--g_cursor_col;
			}
			break;

		case KEY_BACKSPACE:
			start = g_cursor_col;
			if (start != 0) {
				memmove(line + start - 1, line + start, sizeof(line) - start + 2);
				g_cursor_col = start - 1;
				vga_puts(line + start - 1);
				vga_set(' ');
				line[g_cursor_col] = '\0';
				g_cursor_col = start - 1;
			}
			break;

		default:
			return 0;
	}

	return 1;
}

static void tty_insert(char c)
{
	uint8_t col = g_cursor_col;

	if (line[col] != '\0' && col < VGA_COLS - 1) {
		if (line[VGA_COLS - 1] != '\0') {
			return;
		}
		memmove(line + col + 1, line + col, sizeof(line) - col - 2);
		line[sizeof(line) - 1] = '\0';
		vga_putLine(line + col);
	}

	tty_set(c);
	if (g_cursor_col < (VGA_COLS - 1)) {
		++g_cursor_col;
	}
}

int8_t tty_update(char *cmd)
{
	static uint8_t last_key = KEY_NONE;
	static uint8_t arcnt = 0;
	uint8_t key;

	if (keyboard_scan() < 0) {
		return -1;
	}

	/* Ignore multiple keys being pressed */
	key = keyboard_keys.keys[0];

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

		vga_resetCursor();

		if (key == KEY_ENTER) {
			strcpy(cmd, line);
			memset(line, '\0', sizeof(line));
			vga_newLine();
			return 1;
		}

		if (tty_handleSpecial(keyboard_keys.mod, key)) {
			return 0;
		}
		else {
			char c = tty_key2ascii(keyboard_keys.mod, key);
			if (c != '\0') {
				tty_insert(c);
				return 0;
			}
		}
	}

	vga_handleCursor();

	return 0;
}
