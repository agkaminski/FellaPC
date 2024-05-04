/* FellaPC Firmaware
 * BASIC interpreter
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#include <errno.h>
#include <stddef.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>

#include "ualloc/ualloc.h"
#include "interpreter.h"
#include "vga.h"
#include "real.h"

static uint16_t line_curr;
static uint16_t line_next;

struct gosub_elem {
	struct gosub_elem *prev;
	uint16_t line;
};

struct gosub_elem *gosub_stack = NULL;

static int8_t intr_collapseExp(real *o, struct token *tstr, struct token **end)
{
	/* TODO */
	*end = tstr->next;
	return 0;
}

static int8_t intr_isInteger(struct token *token)
{
	uint8_t pos = 0;

	if ((token == NULL) || (token->type != token_real) || (token->value == NULL)) {
		return 0;
	}

	while (token->value[pos] != '\0') {
		if (!isdigit(token->value[pos])) {
			return 0;
		}
		++pos;
	}

	return 1;
}

static int8_t intr_var(struct token *tstr)
{

}

static int8_t intr_print(struct token *tstr)
{
	int8_t first = 1, err;
	char buff[20];
	real r;
	struct token *next = NULL;

	while (1) {
		switch (tstr->type) {
			case token_str:
				vga_puts(tstr->value);
				break;

			case token_real:
			case token_var:
				err = intr_collapseExp(&r, tstr->next, &next);
				if (err < 0) {
					return -EINVAL;
				}
				real_rtoa(buff, &r);
				vga_puts(buff);
				break;

			default:
				if (!first) {
					switch (tstr->type) {
						case token_coma:
							vga_putc('\t');
							break;

						case token_semicol:
							vga_putc(' ');
							break;

						default:
							return -EINVAL;
					}
					break;
				}
				return -EINVAL;
		}

		first = 0;

		if (tstr->next == NULL) {
			vga_putc('\n');
			return 0;
		}
		tstr = (next != NULL) ? next : tstr->next;
		next = NULL;
	}
}

static int8_t intr_input(struct token *tstr)
{

}

static int8_t intr_for(struct token *tstr)
{

}

static int8_t intr_next(struct token *tstr)
{

}

static int8_t intr_goto(struct token *tstr)
{
	if (!intr_isInteger(tstr) || (tstr->next != NULL)) {
		return -EINVAL;
	}

	line_next = atoi(tstr->value);
}

static int8_t intr_if(struct token *tstr)
{
	int8_t condition, err;
	real c;

	err = intr_collapseExp(&c, tstr, &tstr);
	if (err < 0) {
		return -EINVAL;
	}

	condition = !real_isZero(&c);

	if (tstr->type != token_then) {
		return -EINVAL;
	}
	tstr = tstr->next;
	if (!intr_isInteger(tstr)) {
		return -EINVAL;
	}

	if (condition) {
		line_next = atoi(tstr->value);
		return 0;
	}

	if (tstr == NULL) {
		return 0;
	}

	if (tstr->type != token_else) {
		return -1;
	}
	tstr = tstr->next;
	if (!intr_isInteger(tstr) || (tstr->next != NULL)) {
		return -EINVAL;
	}

	line_next = atoi(tstr->value);
	return 0;
}

static int8_t intr_dim(struct token *tstr)
{

}

static int8_t intr_gosub(struct token *tstr)
{
	struct gosub_elem *new;

	if (!intr_isInteger(tstr) || (tstr->next != NULL)) {
		return -EINVAL;
	}

	new = umalloc(sizeof(*new));
	if (new == NULL) {
		return -ENOMEM;
	}

	new->line = line_curr;
	new->prev = NULL;
	if (gosub_stack != NULL) {
		new->prev = gosub_stack;
	}

	gosub_stack = new;

	line_next = atoi(tstr->value);
	return 0;
}

static int8_t intr_return(struct token *tstr)
{
	struct gosub_elem *elem = gosub_stack;

	if ((tstr->next != NULL) || (elem == NULL)) {
		return -EINVAL;
	}

	line_next = elem->line;
	gosub_stack = elem->prev;
	ufree(elem);
}

static int8_t intr_clear(struct token *tstr)
{
	if (tstr->next != NULL) {
		return -EINVAL;
	}

	vga_clear();

	return 0;
}

int8_t interpreter(struct token *tstr)
{
	int8_t (*const entry[])(struct token *) = {
		intr_var,
		intr_print,
		intr_input,
		intr_for,
		intr_next,
		intr_goto,
		intr_if,
		intr_dim,
		intr_gosub,
		intr_return,
		intr_clear
	};

	if (tstr->type >= (sizeof(entry) / sizeof(*entry))) {
		return -EINVAL;
	}

	return entry[tstr->type](tstr->next);
}
