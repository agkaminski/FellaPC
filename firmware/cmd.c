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
#include "token.h"
#include "interpreter.h"

static struct line *line_head = NULL;

jmp_buf cmd_env;

void cmd_die(int8_t err)
{
	longjmp(cmd_env, err);
}

static void cmd_list(void)
{
	char buff[8];
	struct line *curr = line_head;

	while (curr != NULL) {
		utoa(curr->number, buff, 10);
		strcat(buff, " ");
		vga_puts(buff);
		vga_puts(curr->data);
		vga_putc('\n');
		curr = curr->next;
	}
}

static void cmd_new(void)
{
	struct line *curr;

	while (line_head != NULL) {
		curr = line_head;
		line_head = curr->next;
		ufree(curr);
	}

	intr_clean(1);
}

static void cmd_addLine(const char *cmd)
{
	char *end;
	unsigned long number = strtoul(cmd, &end, 10);
	struct line *line, *prev, *curr;

	if (number & 0xffff0000UL) {
		cmd_die(-ERANGE);
	}

	while (isspace(*end)) {
		++end;
	}

	line = umalloc(sizeof(*line));
	if (line == NULL) {
		cmd_die(-ENOMEM);
	}

	line->data = NULL;
	if (*end != '\0') {
		line->data = umalloc(strlen(end) + 1);
		if (line->data == NULL) {
			ufree(line);
			cmd_die(-ENOMEM);
		}
		strcpy(line->data, end);
	}

	line->number = number;

	if ((line_head == NULL) && (line->data != NULL)) {
		line_head = line;
		line->next = NULL;
		return;
	}

	curr = line_head;
	prev = NULL;

	while (curr != NULL) {
		if (curr->number == line->number) {
			ufree(curr->data);
			curr->data = line->data;
			if (line->data == NULL) {
				if (prev != NULL) {
					prev->next = curr->next;
				}
				else {
					line_head = curr->next;
				}
				ufree(curr);
			}
			return;
		}

		if (curr->number > line->number) {
			break;
		}

		prev = curr;
		curr = curr->next;
	}

	if (line->data == NULL) {
		ufree(line);
	}
	else {
		if (prev != NULL) {
			prev->next = line;
		}
		else {
			line_head = line;
		}
		line->next = curr;
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
		else if (*cmd != '\0') {
			intr_line(cmd);
		}
	}

	return err;
}
