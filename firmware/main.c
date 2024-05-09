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
#include "real.h"

static uint8_t heap[28 * 1024];
static const char prompt[] = "Ready\n";
static char cmd[VGA_COLS + 1];

int main(void)
{
	int8_t err;
	char buff[16] = "error ";

	vga_selectRom(0);
	vga_clear();

	ualloc_init(heap, sizeof(heap));

	vga_puts(prompt);

	while (1) {
		vga_vsync();
		if (tty_update(cmd) > 0) {
			err = cmd_parse(cmd);
			if (err < 0) {
				itoa(-err, buff + 6, 10);
				vga_puts(buff);
				vga_putc('\n');
			}
			else if (err == 0) {
				vga_putc('\n');
				vga_puts(prompt);
			}
		}
	}

	return 0;
}
