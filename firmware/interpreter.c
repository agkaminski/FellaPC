/* FellaPC Firmaware
 * BASIC interpreter
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#include <errno.h>
#include <stddef.h>

#include "interpreter.h"
#include "vga.h"
#include "real.h"

static int8_t intr_collapseExp(real *o, struct token *tstr)
{

}

static int8_t intr_var(struct token *tstr)
{

}

static int8_t intr_print(struct token *tstr)
{
	int8_t first = 1, err;
	char buff[20];
	real r;

	while (1) {
		switch (tstr->type) {
			case token_str:
				vga_puts(tstr->value);
				break;

			case token_real:
			case token_var:
				err = intr_collapseExp(&r, tstr);
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
		tstr = tstr->next;
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

}

static int8_t intr_if(struct token *tstr)
{

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
