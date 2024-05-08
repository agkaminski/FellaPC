/* FellaPC Firmaware
 * Keyboard
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "keys.h"
#include "keyboard.h"

extern void keyboard_scanLow(uint8_t *row);

struct keys keyboard_keys = { 0 };

const uint8_t keymap_main[5][16] = {
{
	KEY_ESC, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
	KEY_MINUS, KEY_EQUAL, KEY_BACKSPACE, KEY_HOME
},
{
	KEY_TAB, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P,
	KEY_LEFTBRACE, KEY_RIGHTBRACE, KEY_BACKSLASH, KEY_DELETE
},
{
	KEY_CAPSLOCK, KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L,
	KEY_SEMICOLON, KEY_APOSTROPHE, KEY_NONE, KEY_ENTER, KEY_PAGEUP
},
{
	KEY_LEFTSHIFT, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M, KEY_COMMA, KEY_DOT,
	KEY_SLASH, KEY_NONE, KEY_RIGHTSHIFT, KEY_UP, KEY_PAGEDOWN
},
{
	KEY_LEFTCTRL, KEY_LEFTMETA, KEY_LEFTALT, KEY_NONE, KEY_NONE, KEY_SPACE, KEY_NONE,
	KEY_NONE, KEY_RIGHTALT, KEY_NONE, KEY_RIGHTCTRL, KEY_FN, KEY_LEFT, KEY_DOWN, KEY_RIGHT
} };

/* Row 0 only */
const uint8_t keymap_alt[16] = { KEY_GRAVE, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5,
	KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, KEY_BACKSPACE, KEY_END };

static uint8_t getModifier(uint8_t key)
{
	if ((key < KEY_LEFTCTRL) || (key == KEY_FN)) {
		return 0;
	}

	return 1 << (key & 0xf);
}

int8_t keyboard_scan(void)
{
	/* Check Fn key before, so we know what to do */
	int8_t isFn;
	static uint8_t lastState[16];
	uint8_t row[16];
	uint8_t i;

	keyboard_scanLow(row);

	isFn = (row[9] & (1 << 4));

	for (i = 0; i < 16; ++i) {
		uint8_t change = row[i] ^ lastState[i];
		lastState[i] = row[i];

		if (change) {
			uint8_t j;
			for (j = 0; j < 5; ++j) {
				if (change & (1 << j)) {
					uint8_t key = keymap_main[j][i];
					uint8_t modifier = getModifier(key);
					uint8_t state = row[i] & (1 << j);

					if (isFn && !j) {
						key = keymap_alt[i];
					}

					if (modifier) {
						if (state)
							keyboard_keys.mod |= modifier;
						else
							keyboard_keys.mod &= ~modifier;
					}
					else {
						if (!state) {
							key = KEY_NONE;
						}

						if (state && (keyboard_keys.key != KEY_NONE)) {
							/* Overflow - reset everything to allow reliable recovery */
							memset(&keyboard_keys, 0, sizeof(keyboard_keys));
							memset(lastState, 0, sizeof(lastState));
							return -1;
						}

						keyboard_keys.key = key;
					}
				}
			}
		}
	}

	return 0;
}
