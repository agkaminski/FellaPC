/* FellaPC Firmaware
 * BASIC interpreter
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#include <errno.h>

#include "interpreter.h"
#include "vga.h"

static int8_t intr_var(struct token *tstr)
{

}

static int8_t intr_print(struct token *tstr)
{

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
