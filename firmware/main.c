/* FellaPC Firmaware
 * main
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#include <stdio.h>

#include "vga.h"

int main(void)
{
	int row, col, wait;
	uint8_t offset = 0;

	while (1) {
		for (row = 0; row < 60; ++row) {
			for (col = 0; col < 80; ++col) {
				vga_write(row, col, ' ' + col + row + offset);
			}
		}

		offset++;

		for (wait = 0; wait < 32000; ++wait);
	}

	return 0;
}
