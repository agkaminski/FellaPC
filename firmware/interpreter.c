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
#include "keyboard.h"
#include "tty.h"

static uint16_t line_curr;
static uint16_t line_next;

static struct {
	uint16_t line;
	int8_t jump;
} jump;

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

static struct token *token_curr;

static int8_t interactive = 1;

static void intr_die(int err) __attribute__ ((noreturn));

static void intr_die(int err)
{
	if (!interactive) {
		char buff[8];
		vga_puts("\nLine ");
		itoa(line_curr, buff, 10);
		vga_puts(buff);
		vga_puts(": ");

		intr_clean(0);
	}

	cmd_die(err);
}

static void *intr_malloc(uint8_t size)
{
	void *ret = umalloc(size);
	if (ret == NULL) {
		intr_die(-ENOMEM);
	}
	return ret;
}

static void intr_expect(enum token_type tok)
{
	if (tok == token_none) {
		if (token_curr != NULL) {
			intr_die(-EINVAL);
		}
	}
	else if ((token_curr == NULL) || (token_curr->type != tok)) {
		intr_die(-EINVAL);
	}
}

static void intr_expectInteger(void)
{
	uint8_t pos = 0;

	intr_expect(token_real);

	while (token_curr->value[pos] != '\0') {
		if (!isdigit(token_curr->value[pos])) {
			intr_die(-EINVAL);
		}
		++pos;
	}
}

static void intr_toktor(real *o)
{
	if (real_ator(token_curr->value, o) == NULL) {
		intr_die(-ERANGE);
	}
}

static struct variable *intr_getVar(const char *name, int8_t create)
{
	struct variable *var;
	char *vname;

	var = variables;
	while (var != NULL) {
		if (strcasecmp(var->name, name) == 0) {
			return var;
		}
		var = var->next;
	}

	if (!create) {
		intr_die(-ENOENT);
	}

	var = intr_malloc(sizeof(struct variable));
	vname = intr_malloc(strlen(name) + 1);

	strcpy(vname, name);
	var->name = vname;
	memcpy(&var->val, &rzero, sizeof(real));

	var->next = variables;
	variables = var;

	return var;
}

static struct variable *intr_getTokVar(void)
{
	intr_expect(token_var);
	return intr_getVar(token_curr->value, 1);
}

static void intr_collapseExp(real *o)
{
	struct variable *var;

	/* TODO */

	if (token_curr->type == token_var) {
		var = intr_getTokVar();
		memcpy(o, &var->val, sizeof(*o));
	}
	else if (token_curr->type == token_real) {
		intr_toktor(o);
	}

	/* Need to eat all relevant tokens */
	token_curr = token_curr->next;
}

static void intr_var(void)
{
	struct variable *var;

	token_curr = token_curr->prev;

	var = intr_getTokVar();

	token_curr = token_curr->next;
	intr_expect(token_eq);
	token_curr = token_curr->next;

	intr_collapseExp(&var->val);

	intr_expect(token_none);
}

static void intr_print(void)
{
	int8_t first = 1;
	char buff[20];
	real r;

	while (token_curr != NULL) {
		switch (token_curr->type) {
			case token_str:
				vga_puts(token_curr->value);
				break;

			case token_real:
			case token_var:
				intr_collapseExp(&r);
				real_rtoa(buff, &r);
				vga_puts(buff);
				continue;

			default:
				if (!first) {
					switch (token_curr->type) {
						case token_coma:
							vga_putc('\t');
							break;

						case token_semicol:
							vga_putc(' ');
							break;

						default:
							intr_die(-EINVAL);
					}
					break;
				}
				intr_die(-EINVAL);
		}

		first = 0;

		token_curr = token_curr->next;
	}

	vga_putc('\n');
}

static void intr_input(void)
{
	const char *prompt = "?";
	struct variable *var;
	char cmd[81];

	if (token_curr == NULL) {
		intr_die(-EINVAL);
	}

	if (token_curr->type == token_str) {
		prompt = token_curr->value;

		token_curr = token_curr->next;
		intr_expect(token_coma);
		token_curr = token_curr->next;
	}

	var = intr_getTokVar();

	token_curr = token_curr->next;
	intr_expect(token_none);

	vga_puts(prompt);
	vga_putc('\n');

	do {
		vga_vsync();
	} while (tty_update(cmd) <= 0);

	/* TODO add strings */

	real_ator(cmd, &var->val);
}

static void intr_for(void)
{
	struct variable *iter;
	struct for_elem *f;
	real limit, step;

	memcpy(&step, &rone, sizeof(real));

	iter = intr_getTokVar();

	f = for_stack;
	while (f != NULL) {
		if (f->iter == iter) {
			intr_die(-EINVAL);
		}
		f = f->next;
	}

	token_curr = token_curr->next;
	intr_expect(token_eq);

	token_curr = token_curr->next;
	intr_collapseExp(&iter->val);

	intr_expect(token_to);

	token_curr = token_curr->next;
	intr_collapseExp(&limit);

	if (token_curr != NULL) {
		intr_expect(token_step);

		token_curr = token_curr->next;
		intr_collapseExp(&step);

		intr_expect(token_none);
	}

	f = intr_malloc(sizeof(*f));
	f->line = line_next;
	f->iter = iter;
	f->limit = limit;
	f->step = step;
	f->next = for_stack;
	for_stack = f;
}

static void intr_next(void)
{
	struct variable *var;
	struct for_elem *f = for_stack, *prev = NULL;
	int8_t err;
	real acc;

	var = intr_getTokVar();

	token_curr = token_curr->next;
	intr_expect(token_none);

	while (f != NULL) {
		if (f->iter == var) {
			err = real_add(&acc, &f->iter->val, &f->step);
			if (err < 0) {
				intr_die(err);
			}
			memcpy(&f->iter->val, &acc, sizeof(real));

			err = real_sub(&acc, &f->limit, &f->iter->val);
			if (acc.s > 0) {
				jump.line = f->line;
				jump.jump = 1;
			}
			else {
				if (prev != NULL) {
					prev->next = f->next;
				}
				else {
					for_stack = f->next;
				}
				ufree(f);
			}
			break;
		}
		prev = f;
		f = f->next;
	}
}

static void intr_goto(void)
{
	intr_expectInteger();

	jump.line = atoi(token_curr->value);
	jump.jump = 1;

	token_curr = token_curr->next;
	intr_expect(token_none);
}

static void intr_if(void)
{
	int8_t condition;
	real c;

	intr_collapseExp(&c);
	condition = !real_isZero(&c);

	intr_expect(token_then);

	token_curr = token_curr->next;
	intr_expectInteger();

	if (condition) {
		jump.line = atoi(token_curr->value);
		jump.jump = 1;
		return;
	}

	token_curr = token_curr->next;
	if (token_curr != NULL) {
		intr_expect(token_else);

		token_curr = token_curr->next;
		intr_expectInteger();

		jump.line = atoi(token_curr->value);
		jump.jump = 1;

		token_curr = token_curr->next;
		intr_expect(token_none);
	}
}

static void intr_dim(void)
{
	/* TODO */
	intr_die(-ENOSYS);
}

static void intr_gosub(void)
{
	struct gosub_elem *new;

	intr_expectInteger();

	new = intr_malloc(sizeof(*new));
	new->line = line_curr;
	new->prev = gosub_stack;

	gosub_stack = new;

	jump.line = atoi(token_curr->value);
	jump.jump = 1;

	token_curr = token_curr->next;
	intr_expect(token_none);
}

static void intr_return(void)
{
	struct gosub_elem *elem = gosub_stack;

	intr_expect(token_none);

	if (elem == NULL) {
		intr_die(-EINVAL);
	}

	jump.line = elem->line;
	jump.jump = 1;

	gosub_stack = elem->prev;
	ufree(elem);
}

static void intr_clear(void)
{
	intr_expect(token_none);

	vga_clear();
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

	jump.jump = 0;

	if (hard) {
		while (variables != NULL) {
			struct variable *victim = variables;
			variables = victim->next;
			ufree(victim->name);
			ufree(victim);
		}
	}

	line_curr = 0;
	interactive = 1;
}

void intr_line(const char *line)
{
	int8_t err;
	struct token *tstr;
	static void (*const entry[])(void) = {
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
	if (err < 0) {
		intr_die(err);
	}

	if (tstr->type >= (sizeof(entry) / sizeof(*entry))) {
		token_free(tstr);
		intr_die(-EINVAL);
	}

	token_curr = tstr->next;

	entry[tstr->type]();

	token_free(tstr);
}

void intr_run(struct line *start)
{
	struct line *curr = start;

	interactive = 0;

	while (curr != NULL) {
		if (keyboard_scan() >= 0) {
			if (keyboard_keys.key == KEY_ESC) {
				break;
			}
		}

		line_curr = curr->number;
		line_next = (curr->next != NULL) ? curr->next->number : 0xffffu;

		intr_line(curr->data);

		if (jump.jump) {
			jump.jump = 0;
			curr = start;
			while ((curr != NULL) && (curr->number != jump.line)) {
				curr = curr->next;
			}
		}
		else {
			curr = curr->next;
		}
	}

	intr_clean(0);
}
