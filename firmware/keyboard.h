/* FellaPC Firmaware
 * Keyboard
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include <stdint.h>

struct keys {
	uint8_t mod;
	uint8_t keys[6];
};

int keyboard_scan(struct keys *keys);

#endif
