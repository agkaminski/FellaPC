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
#include <string.h>

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

struct variable {
	struct variable *next;
	const char *name;
	real val;
};

static struct variable *variables = NULL;

struct for_elem {
	struct for_elem *next;
	uint16_t line;
	struct variable *iter;
	real limit;
	real step;
};

struct for_elem *for_stack = NULL;

static int8_t intr_getVar(struct variable **var, const char *name, int8_t create)
{
	(*var) = variables;
	while ((*var) != NULL) {
		if (strcasecmp((*var)->name, name) == 0) {
			return 0;
		}
		(*var) = (*var)->next;
	}

	if (create) {
		char *vname;
		(*var) = umalloc(sizeof(struct variable));
		if ((*var) == NULL) {
			return -ENOMEM;
		}

		vname = umalloc(strlen(name) + 1);
		if (vname == NULL) {
			ufree(*var);
			return -ENOMEM;
		}

		strcpy(vname, name);
		(*var)->name = vname;
		memcpy(&(*var)->val, &rzero, sizeof(real));

		(*var)->next = variables;
		variables = (*var);

		return 0;
	}

	return -ENOENT;
}

static int8_t intr_collapseExp(real *o, struct token *tstr, struct token **end)
{
	struct variable *var;
	int8_t err = -EINVAL;

	/* TODO */

	if (tstr->type == token_var) {
		err = intr_getVar(&var, tstr->value, 0);
		if (err < 0) {
			return err;
		}
		memcpy(o, &var->val, sizeof(*o));
	}
	else if (tstr->type == token_real) {
		err = (real_ator(tstr->value, o) == NULL) ? -ERANGE : 0;
	}

	*end = tstr->next;

	return err;
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
	struct variable *var;
	int8_t err;

	err = intr_getVar(&var, tstr->value, 1);
	if (err < 0) {
		return err;
	}

	if ((tstr->next == NULL) || (tstr->next->type != token_eq)) {
		return -EINVAL;
	}

	tstr = tstr->next->next;

	err = intr_collapseExp(&var->val, tstr, &tstr);
	if (err < 0) {
		return err;
	}

	if (tstr != NULL) {
		return -EINVAL;
	}

	return 0;
}

static int8_t intr_print(struct token *tstr)
{
	int8_t first = 1, err;
	char buff[20];
	real r;
	struct token *next = NULL;

	tstr = tstr->next;

	while (tstr != NULL) {
		switch (tstr->type) {
			case token_str:
				vga_puts(tstr->value);
				break;

			case token_real:
			case token_var:
				err = intr_collapseExp(&r, tstr, &next);
				if (err < 0) {
					return err;
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

		tstr = (next != NULL) ? next : tstr->next;
		next = NULL;
	}

	vga_putc('\n');
	return 0;
}

static int8_t intr_input(struct token *tstr)
{
	/* TODO */
	return -ENOSYS;
}

static int8_t intr_for(struct token *tstr)
{
	struct variable *iter;
	struct for_elem *f;
	real limit, step;
	int8_t err;

	memcpy(&step, &rone, sizeof(real));

	tstr = tstr->next;
	if (tstr->type != token_var) {
		return -EINVAL;
	}

	err = intr_getVar(&iter, tstr->value, 1);
	if (err < 0) {
		return err;
	}

	tstr = tstr->next;
	if (tstr->type != token_eq) {
		return -EINVAL;
	}

	tstr = tstr->next;
	err = intr_collapseExp(&iter->val, tstr, &tstr);
	if (err < 0) {
		return err;
	}

	if (tstr->type != token_to) {
		return -EINVAL;
	}

	tstr = tstr->next;
	err = intr_collapseExp(&limit, tstr, &tstr);
	if (err < 0) {
		return err;
	}

	if (tstr != NULL) {
		if (tstr->type != token_step) {
			return -EINVAL;
		}

		tstr = tstr->next;
		err = intr_collapseExp(&step, tstr, &tstr);
		if (err < 0) {
			return err;
		}

		if (tstr != NULL) {
			return -EINVAL;
		}
	}

	f = umalloc(sizeof(*f));
	if (f == NULL) {
		return -ENOMEM;
	}

	f->line = line_next;
	f->iter = iter;
	f->limit = limit;
	f->step = step;
	f->next = for_stack;
	for_stack = f;

	return 0;
}

static int8_t intr_next(struct token *tstr)
{
	struct variable *var;
	struct for_elem *f = for_stack, *prev = NULL;
	int8_t err;
	real acc;

	tstr = tstr->next;
	if (tstr->type != token_var) {
		return -EINVAL;
	}

	err = intr_getVar(&var, tstr->value, 0);
	if (err < 0) {
		return err;
	}

	if (tstr->next != NULL) {
		return -EINVAL;
	}

	while (f != NULL) {
		if (f->iter == var) {
			err = real_add(&acc, &f->iter->val, &f->step);
			if (err < 0) {
				return err;
			}
			memcpy(&f->iter->val, &acc, sizeof(real));

			err = real_sub(&acc, &f->limit, &f->iter->val);
			if (acc.s > 0) {
				line_next = f->line;
				if (prev != NULL) {
					prev->next = f->next;
				}
				else {
					for_stack = f->next;
				}
				ufree(f);
			}
			return 0;
		}
		prev = f;
		f = f->next;
	}

	return -EINVAL;
}

static int8_t intr_goto(struct token *tstr)
{
	tstr = tstr->next;

	if (!intr_isInteger(tstr) || (tstr->next != NULL)) {
		return -EINVAL;
	}

	line_next = atoi(tstr->value);
}

static int8_t intr_if(struct token *tstr)
{
	int8_t condition, err;
	real c;

	tstr = tstr->next;

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
	/* TODO */
	return -ENOSYS;
}

static int8_t intr_gosub(struct token *tstr)
{
	struct gosub_elem *new;

	tstr = tstr->next;

	if (!intr_isInteger(tstr) || (tstr->next != NULL)) {
		return -EINVAL;
	}

	new = umalloc(sizeof(*new));
	if (new == NULL) {
		return -ENOMEM;
	}

	new->line = line_curr;

	new->prev = gosub_stack;
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

void intr_clean(int8_t hard)
{
	while (gosub_stack != NULL) {
		struct gosub_elem *victim = gosub_stack;
		gosub_stack = victim->prev;
		ufree(victim);
	}

	while (for_stack != NULL) {
		struct for_elem *victim = for_stack;
		for_stack = victim->next;
		ufree(victim);
	}

	if (hard) {
		while (variables != NULL) {
			struct variable *victim = variables;
			variables = victim->next;
			ufree(victim->name);
			ufree(victim);
		}
	}
}

int8_t intr_line(const char *line)
{
	int8_t err;
	struct token *tstr;
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

	err = token_tokenize(&tstr, line);
	if (err >= 0) {
		err = -EINVAL;
		if (tstr->type < (sizeof(entry) / sizeof(*entry))) {
			err = entry[tstr->type](tstr);
		}
	}

	token_free(tstr);

	return err;
}

int8_t intr_run(struct line *start)
{

}
