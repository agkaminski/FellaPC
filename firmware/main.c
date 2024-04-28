/* FellaPC Firmaware
 * main
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#include <stdio.h>

#include "vga.h"
#include "keyboard.h"
#include "tty.h"

int main(void)
{
	//vga_selectRom(0);
	vga_clear();

/*
	while (1) {
		static struct keys keys = { 0 };
		char buff[80];
		long wait;
		int ret;

		ret = keyboard_scan(&keys);
		sprintf(buff, "Keys: 0x%02x %.3hhu %.3hhu %.3hhu %.3hhu %.3hhu %.3hhu (%d)\n",
			keys.mod, keys.keys[0], keys.keys[1], keys.keys[2],
			keys.keys[3], keys.keys[4], keys.keys[5], ret);
		vga_puts(buff);
		for (wait = 0; wait < 1000; ++wait);
	}
*/

	while (1) {
		char cmd[80];
		vga_vsync();
		if (tty_update(cmd) > 0) {
			vga_puts("CMD: ");
			vga_puts(cmd);
			vga_putc('\n');
		}
	}

	return 0;
}
