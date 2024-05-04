/* FellaPC Firmaware
 * Command handling
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#ifndef CMD_H_
#define CMD_H_

struct line {
	uint16_t number;
	struct line *next;
	char *data;
};

int8_t cmd_parse(const char *cmd);

#endif
