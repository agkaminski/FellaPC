#include <stdint.h>
#include <stddef.h>

#include "keys.h"
#include "keyboard.h"

#define K(main, alt) (((alt) << 8) | ((main) & 0xff))
#define MAIN(key) ((key) & 0xff)
#define ALT(key) (((key) >> 8) & 0xff) 

static uint8_t * const base = (void *)0x9000;

static uint8_t lastState[16];

static const uint16_t map[5][16] = {
{
	K(KEY_ESC, KEY_GRAVE), K(KEY_1, KEY_F1), K(KEY_2, KEY_F2),
	K(KEY_3, KEY_F3), K(KEY_4, KEY_F4), K(KEY_5, KEY_F5), K(KEY_6, KEY_F6),
	K(KEY_7, KEY_F7), K(KEY_8, KEY_F8), K(KEY_9, KEY_F9), K(KEY_0, KEY_F10),
	K(KEY_MINUS, KEY_F11), K(KEY_EQUAL, KEY_F12),
	K(KEY_BACKSPACE, KEY_BACKSPACE), K(KEY_DELETE, KEY_DELETE)
},
{
	K(KEY_TAB, KEY_TAB), K(KEY_Q, KEY_Q), K(KEY_W, KEY_W), K(KEY_E, KEY_E),
	K(KEY_R, KEY_R), K(KEY_T, KEY_T), K(KEY_Y, KEY_Y), K(KEY_U, KEY_U),
	K(KEY_I, KEY_I), K(KEY_O, KEY_O), K(KEY_P, KEY_P),
	K(KEY_LEFTBRACE, KEY_LEFTBRACE), K(KEY_RIGHTBRACE, KEY_RIGHTBRACE),
	K(KEY_BACKSLASH, KEY_BACKSLASH), K(KEY_PAGEUP, KEY_PAGEUP)
},
{
	K(KEY_CAPSLOCK, KEY_CAPSLOCK), K(KEY_A, KEY_A), K(KEY_S, KEY_S),
	K(KEY_D, KEY_D), K(KEY_F, KEY_F), K(KEY_G, KEY_G), K(KEY_H, KEY_H),
	K(KEY_J, KEY_J), K(KEY_K, KEY_K), K(KEY_L, KEY_L),
	K(KEY_SEMICOLON, KEY_SEMICOLON), K(KEY_APOSTROPHE, KEY_APOSTROPHE),
	K(KEY_NONE, KEY_NONE), K(KEY_ENTER, KEY_ENTER),
	K(KEY_PAGEDOWN, KEY_PAGEDOWN)
},
{
	K(KEY_LEFTSHIFT, KEY_LEFTSHIFT), K(KEY_Z, KEY_Z), K(KEY_X, KEY_X),
	K(KEY_C, KEY_C), K(KEY_V, KEY_V), K(KEY_B, KEY_B), K(KEY_N, KEY_N),
	K(KEY_M, KEY_M), K(KEY_COMMA, KEY_COMMA), K(KEY_DOT, KEY_DOT),
	K(KEY_NONE, KEY_NONE), K(KEY_RIGHTSHIFT, KEY_RIGHTSHIFT),
	K(KEY_UP, KEY_UP), K(KEY_HOME, KEY_END)
},
{
	K(KEY_LEFTCTRL, KEY_LEFTCTRL), K(KEY_LEFTMETA, KEY_LEFTMETA),
	K(KEY_LEFTALT, KEY_LEFTALT), K(KEY_NONE, KEY_NONE),
	K(KEY_NONE, KEY_NONE), K(KEY_SPACE, KEY_SPACE),
	K(KEY_NONE, KEY_NONE), K(KEY_NONE, KEY_NONE),
	K(KEY_RIGHTALT, KEY_RIGHTALT), K(KEY_NONE, KEY_NONE),
	K(KEY_RIGHTCTRL, KEY_RIGHTCTRL), K(KEY_FN, KEY_FN), K(KEY_LEFT, KEY_LEFT),
	K(KEY_DOWN, KEY_DOWN), K(KEY_RIGHT, KEY_RIGHT)
} };

static int keyPress(struct keys *keys, uint8_t key)
{
	size_t i;
	/* Find first free place */
	for (i = 0; i < sizeof(keys->keys) / sizeof(keys->keys[0]); ++i) {
		if (keys->keys[i] == KEY_NONE) {
			keys->keys[i] = key;
			return 0;
		}
	}

	/* Overflow */
	return -1;
}

static int keyRelease(struct keys *keys, uint8_t key)
{
	size_t i;

	for (i = 0; i < sizeof(keys->keys) / sizeof(keys->keys[0]); ++i) {
		if (keys->keys[i] == key) {
			/* Remove and compress */
			for (; i < sizeof(keys->keys) / sizeof(keys->keys[0]) - 1; ++i) {
				keys->keys[i] = keys->keys[i + 1];
			}

			keys->keys[i] = KEY_NONE;

			return 0;
		}
	}

	/* Key not found? */
	return -1;
}

int keyboard_scan(struct keys *keys)
{
	/* Check Fn key before, so we know what to do */
	int isFn = !(base[11] & (1 << 4));

	uint8_t i;
	for (i = 0; i < 16; ++i) {
		uint8_t row = ~base[i];
		uint8_t change = row ^ lastState[i];
		
		lastState[i] = row;
		
		if (change) {
			uint8_t j;
			for (j = 0; j < 5; ++j) {
				if (change & (1 << j)) {
					uint8_t key = isFn ? ALT(map[j][i]) : MAIN(map[j][i]);
					int ret = (row & (1 << j)) ?
						keyPress(keys, key) : keyRelease(keys, key);

					if (ret < 0) {
						/* Overflow */
						return ret;
					}
				}
			}
		}
	}

	return 0;
}

