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

#include "ualloc/ualloc.h"
#include "vga.h"
#include "token.h"
#include "interpreter.h"

struct line {
	uint16_t number;
	struct line *next;
	char *data;
};

static struct line *line_head = NULL;

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
	struct line *curr = line_head, *next;

	while (curr != NULL) {
		next = curr->next;
		ufree(curr->data);
		ufree(curr);
		curr = next;
	}

	line_head = NULL;

	intr_clean(1);
}

static int8_t cmdd_addLine(const char *cmd)
{
	char *end;
	unsigned long number = strtoul(cmd, &end, 10);
	struct line *line, *prev, *curr;

	if (end == NULL) {
		return -EINVAL;
	}

	if ((number & 0xffffUL) != number) {
		return -ERANGE;
	}

	while (isspace(*end)) {
		++end;
	}

	line = umalloc(sizeof(*line));
	if (line == NULL) {
		return -ENOMEM;
	}

	line->data = NULL;
	if (*end != '\0') {
		line->data = umalloc(strlen(end) + 1);
		if (line->data == NULL) {
			ufree(line);
			return -ENOMEM;
		}
		strcpy(line->data, end);
	}

	line->number = number;

	if ((line_head == NULL) && (line->data != NULL)) {
		line_head = line;
		line->next = NULL;
		return 0;
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
			break;
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

	return 0;
}

int8_t cmd_parse(const char *cmd)
{
	int8_t err = 0;
	struct token *tstr;

	if (isdigit(cmd[0])) {
		err = cmdd_addLine(cmd);
	}
	else if (strcasecmp(cmd, "list") == 0) {
		cmd_list();
	}
	else if (strcasecmp(cmd, "new") == 0) {
		cmd_new();
	}
	else if (*cmd != '\0') {
		err = token_tokenize(&tstr, cmd);
		if (err >= 0) {
			err = interpreter(tstr);
			token_free(tstr);
		}
	}

	return err;
}
