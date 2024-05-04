/* FellaPC Firmaware
 * BASIC interpreter
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#include <errno.h>
#include <stddef.h>
#include <ctype.h>
#include <stdlib.h>

#include "interpreter.h"
#include "vga.h"
#include "real.h"

static uint16_t next_line;

static int8_t intr_collapseExp(real *o, struct token *tstr, struct token **end)
{
	/* TODO */
	*end = tstr->next;
	return 0;
}

static int8_t intr_jump(const char *number)
{
	uint8_t pos = 0;

	while (number[pos] != '\0') {
		if (!isdigit(number[pos])) {
			return -EINVAL;
		}
		++pos;
	}

	next_line = atoi(number);

	return 0;
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
	if ((tstr->type == token_real) || (tstr->next != NULL)) {
		return intr_jump(tstr->value);
	}

	return -EINVAL;
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
	if ((tstr == NULL) || (tstr->type != token_real)) {
		return -EINVAL;
	}

	if (condition) {
		return intr_jump(tstr->value);
	}

	if (tstr == NULL) {
		return 0;
	}

	if (tstr->type != token_else) {
		return -1;
	}
	tstr = tstr->next;
	if ((tstr == NULL) || (tstr->type != token_real) || (tstr->next != NULL)) {
		return -EINVAL;
	}

	return intr_jump(tstr->value);
}

static int8_t intr_dim(struct token *tstr)
{

}

static int8_t intr_gosub(struct token *tstr)
{

}

static int8_t intr_return(struct token *tstr)
{

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
