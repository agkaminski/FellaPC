/* FellaPC Firmaware
 * Command handling
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <setjmp.h>

#include "cmd.h"
#include "ualloc/ualloc.h"
#include "vga.h"
#include "basic/token.h"
#include "basic/interpreter.h"
#include "basic/list.h"
#include "i2c.h"

static struct line *line_head = NULL;

jmp_buf cmd_env;

void cmd_die(int8_t err)
{
	longjmp(cmd_env, err);
}

static size_t cmd_sprintLine(char *buff, const struct line *line)
{
	utoa(line->number, buff, 10);
	strcat(buff, " ");
	strcat(buff, line->data);
	strcat(buff, "\n");
	return strlen(buff);
}

static void cmd_list(void)
{
	char buff[VGA_COLS + 1];
	struct line *curr = line_head;

	while (curr != NULL) {
		cmd_sprintLine(buff, curr);
		vga_puts(buff);
		curr = curr->next;
	}
}

static void cmd_new(void)
{
	list_ufree(&line_head);
	intr_clean(1);
}

static void cmd_addLine(const char *cmd)
{
	/* strtoul would be much better, but can't afford it */
	int number = atoi(cmd);
	struct line *line = NULL, *prev, *curr;
	size_t len;

	if (number < 0) {
		cmd_die(-ERANGE);
	}

	while (isdigit(*cmd) || isspace(*cmd)) {
		++cmd;
	}

	len = strlen(cmd);

	if (len) {
		line = umalloc(sizeof(*line) + len + 1);
		if (line == NULL) {
			cmd_die(-ENOMEM);
		}
		strcpy(line->data, cmd);
		line->number = number;
	}

	curr = line_head;
	prev = NULL;

	while (curr != NULL) {
		if (curr->number == number) {
			struct line *next = curr->next;
			if (prev != NULL) {
				prev->next = curr->next;
			}
			else {
				line_head = curr->next;
			}
			ufree(curr);
			curr = next;
			break;
		}

		if (curr->number > number) {
			break;
		}

		prev = curr;
		curr = curr->next;
	}

	if (line != NULL) {
		if (prev != NULL) {
			prev->next = line;
		}
		else {
			line_head = line;
		}
		line->next = curr;
	}
}

static void cmd_save(void)
{
	char buff[VGA_COLS + 1];
	struct line *curr = line_head;
	uint16_t address = 0;
	size_t len;
	static const char term = '\0';

	while (curr != NULL) {
		len = cmd_sprintLine(buff, curr);
		i2c_write(address, buff, len);
		curr = curr->next;
		address += len;
	}
	i2c_write(address, &term, 1);
}

static void cmd_load(void)
{
	char buff[VGA_COLS + 1];
	char c;
	uint16_t address = 0;
	uint8_t pos = 0;

	while (1) {
		i2c_read(address, &c, 1);
		if ((c == '\0') || (c == 0xff)) {
			break;
		}

		if (c == '\n') {
			buff[pos] = '\0';
			cmd_addLine(buff);
			pos = 0;
		}
		else {
			buff[pos++] = c;
		}

		++address;
	}
}

int8_t cmd_parse(const char *cmd)
{
	int8_t err;

	err = setjmp(cmd_env);
	if (!err) {
		if (isdigit(cmd[0])) {
			cmd_addLine(cmd);
			err = 1; /* Supress prompt */
		}
		else if (strcasecmp(cmd, "list") == 0) {
			cmd_list();
		}
		else if (strcasecmp(cmd, "new") == 0) {
			cmd_new();
		}
		else if (strcasecmp(cmd, "run") == 0) {
			intr_run(line_head);
		}
		else if (strcasecmp(cmd, "save") == 0) {
			cmd_save();
		}
		else if (strcasecmp(cmd, "load") == 0) {
			cmd_load();
		}
		else if (*cmd != '\0') {
			intr_line(cmd);
		}
	}

	return err;
}
