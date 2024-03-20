#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include <stdint.h>

struct keys {
	uint8_t mod;
	uint8_t keys[6];
};

#endif
