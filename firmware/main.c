/* FellaPC Firmaware
 * main
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#include <stdio.h>
#include <stdlib.h>

#include "system.h"
#include "vga.h"
#include "keyboard.h"
#include "tty.h"
#include "ualloc/ualloc.h"
#include "cmd.h"

static const char prompt[] = "Ready\n";
static char cmd[VGA_COLS + 1];

const char *welcome[] = {
	"CLEAR",
	"PRINT\"FellaPC " VERSION "\"",
	"PRINT\"github.com/agkaminski/FellaPC\"",
	"PRINT\"Copyright A.K. 2024\"",
	"PRINT\"BASIC\";FRE;\"bytes free.\"",
	"NEW"
};

int main(void)
{
	int8_t err = 0, i;
	char buff[16] = "error ";

	system_init();

	for (i = 0; i < sizeof(welcome) / sizeof(*welcome); ++i) {
		cmd_parse(welcome[i]);
	}

	while (1) {
		if (err < 0) {
			itoa(-err, buff + 6, 10);
			vga_puts(buff);
			vga_putc('\n');
		}
		else if (err == 0) {
			vga_putc('\n');
			vga_puts(prompt);
		}

		do {
			vga_vsync();
			err = tty_update(cmd);
		} while (err <= 0);

		err = cmd_parse(cmd);
	}

	return 0;
}
