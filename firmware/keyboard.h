/* FellaPC Firmaware
 * Keyboard
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include <stdint.h>

#include "keys.h"

struct keys {
	uint8_t mod;
	uint8_t keys[6];
};

extern struct keys keyboard_keys;

int8_t keyboard_scan(void);

#endif
