/* FellaPC Firmaware
 * main
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#include <stdio.h>
#include <stdlib.h>

#include "vga.h"
#include "keyboard.h"
#include "tty.h"
#include "ualloc/ualloc.h"
#include "cmd.h"

static uint8_t heap[30 * 1024];

#if 0
void dump(uint16_t addr, uint16_t len)
{
	uint16_t i, j;
	char buff[8];
	volatile uint8_t *ptr = (void *)addr;

	for (i = 0; i < len; i += 16) {
		sprintf(buff, "%04X: ", addr + i);
		vga_puts(buff);

		for (j = 0; j < 16 && i + j < len; ++j) {
			sprintf(buff, "%02X ", ptr[i + j]);
			vga_puts(buff);
		}

		if (i + j < len) vga_putc('\n');
	}
}
#endif

int main(void)
{
	int8_t err;
	char buff[16] = "Error ";

	vga_selectRom(0);
	vga_clear();

	ualloc_init(heap, sizeof(heap));

	while (1) {
		char cmd[VGA_COLS + 1];
		vga_vsync();
		if (tty_update(cmd) > 0) {
			err = cmd_parse(cmd);
			if (err < 0) {
				itoa(-err, buff + 6, 10);
				vga_puts(buff);
				vga_putc('\n');
			}
		}
	}

	return 0;
}
