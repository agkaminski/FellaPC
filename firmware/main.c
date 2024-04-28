/* FellaPC Firmaware
 * main
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#include <stdio.h>

#include "vga.h"

int main(void)
{
	char buff[32];
	uint8_t counter = 0;
	long wait;

	vga_clear();

	while (1) {
		sprintf(buff, "This is the line number %d.\n", counter++);
		vga_puts(buff);
		for (wait = 0; wait < 1000; ++wait);
	}

	return 0;
}
