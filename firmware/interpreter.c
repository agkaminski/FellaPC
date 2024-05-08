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
#include "list.h"

static int line_curr;
static int line_next;

static struct {
	int line;
	int8_t jump;
} jump;

struct gosub_elem {
	struct gosub_elem *next;
	struct gosub_elem *prev;
	int line;
};

struct gosub_elem *gosub_stack = NULL;

struct variable {
	struct variable *next;
	struct variable *prev;
	real val;
	const char name[];
};

static struct variable *variables = NULL;

struct for_elem {
	struct for_elem *next;
	struct for_elem *prev;
	int line;
	struct variable *iter;
	real limit;
	real step;
};

struct for_elem *for_stack = NULL;

static struct token *token_curr;

static int8_t interactive = 1;

static struct token *rpn_output = NULL;
static struct token *rpn_opstack = NULL;
static struct token *rpn_stack = NULL;

static void intr_die(int err) __attribute__ ((noreturn));

static void intr_die(int err)
{
	list_ufree(&rpn_output);
	list_ufree(&rpn_opstack);
	list_ufree(&rpn_stack);

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

static void intr_assertNotNull(void *ptr)
{
	if (ptr == NULL) {
		intr_die(-EINVAL);
	}
}

static void *intr_malloc(uint8_t size)
{
	void *ret = umalloc(size);
	intr_assertNotNull(ret);
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

static int intr_expectInteger(void)
{
	uint8_t pos = 0;

	intr_expect(token_real);
	return real_rtoi(&token_curr->value);
}

static struct variable *intr_getVar(const char *name, int8_t create)
{
	struct variable *var;

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

	var = intr_malloc(sizeof(struct variable) + strlen(name) + 1);
	strcpy((char *)var->name, name);

	memcpy(&var->val, &rzero, sizeof(real));

	list_push(&variables, var);

	return var;
}

static struct variable *intr_getTokVar(void)
{
	intr_expect(token_var);
	return intr_getVar(token_curr->str, 1);
}

static uint8_t intr_opPrecedence(enum token_type type)
{
	switch (type) {
		case token_negative:
			return 100;

		case token_mul:
		case token_div:
			return 2;

		case token_plus:
		case token_minus:
			return 1;

		case token_lt:
		case token_lteq:
		case token_gt:
		case token_gteq:
		case token_eq:
			return 0;
	}

	intr_die(-EINVAL);
}

static void intr_shuntingYard(void)
{
	/* Use Edsger Dijkstra's Shunting Yard algorithm to
	 * convert natural expression to easy to calculate
	 * RPN. All tokens related to the expresion are
	 * removed from the token string and shall be freed
	 * after completing the calculation. */

	struct token *curr, *t;

	while ((token_curr != NULL) && (token_curr->type != token_semicol) &&
			(token_isValue(token_curr->type) ||
			(token_curr->type >= TOKEN_OPERATOR_START))) {

		curr = token_curr;
		token_curr = token_curr->next;
		list_pop(NULL, curr); /* NULL's ok, never first element */

		if (curr->type == token_var) {
			struct variable *var = intr_getVar(curr->str, 0);
			curr->type = token_real;
			memcpy(&curr->value, &var->val, sizeof(curr->value));
			list_append(&rpn_output, curr);
		}
		else if (curr->type == token_real) {
			list_append(&rpn_output, curr);
		}
		else if ((curr->type == token_lpara) || (curr->type >= TOKEN_FUNCTION_START)) {
			list_push(&rpn_opstack, curr);
		}
		else if (curr->type == token_coma) {
			while (1) {
				t = rpn_opstack;
				intr_assertNotNull(t);

				if (t->type == token_lpara) {
					break;
				}

				list_pop(&rpn_opstack, t);
				list_append(&rpn_output, t);
			}

			ufree(curr);
		}
		else if (curr->type == token_rpara) {
			while (1) {
				t = rpn_opstack;
				intr_assertNotNull(t);

				list_pop(&rpn_opstack, t);
				if (t->type == token_lpara) {
					break;
				}

				list_append(&rpn_output, t);
			}

			/* Discard left parenthesis */
			ufree(t);

			t = rpn_opstack;
			if ((t != NULL) && (t->type >= TOKEN_FUNCTION_START)) {
				list_pop(&rpn_opstack, t);
				list_append(&rpn_output, t);
			}
		}
		else { /* Operator */
			while ((rpn_opstack != NULL) && (rpn_opstack->type != token_lpara)) {
				int8_t precedence = intr_opPrecedence(rpn_opstack->type) - intr_opPrecedence(curr->type);
				if ((precedence < 0) || (!precedence && (curr->type == token_negative))) {
					break;
				}

				t = rpn_opstack;
				list_pop(&rpn_opstack, t);
				list_append(&rpn_output, t);
			}

			list_push(&rpn_opstack, curr);
		}
	}

	while (rpn_opstack != NULL) {
		t = rpn_opstack;
		if (t->type == token_lpara) {
			intr_die(-EINVAL);
		}
		list_pop(&rpn_opstack, t);
		list_append(&rpn_output, t);
	}
}

static void intr_collapseExp(real *o)
{
	intr_shuntingYard();

	/* FIXME: Calculation WIP */

	while (rpn_output != NULL) {
		struct token *tok = rpn_output;
		list_pop(&rpn_output, tok);

		if (tok->type == token_real) {
			list_push(&rpn_stack, tok);
		}
		else {
			if (tok->type == token_negative) {
				intr_assertNotNull(rpn_stack);
				rpn_stack->value.s = -rpn_stack->value.s;
			}
			else { /* Binary operations */
				struct token *a, *b;
				real r;
				int8_t cmp;

				intr_assertNotNull(rpn_stack);
				b = rpn_stack;

				list_pop(&rpn_stack, b);

				intr_assertNotNull(rpn_stack);
				a = rpn_stack;

				switch (tok->type) {
					case token_mul:
						real_mul(&r, &a->value, &b->value);
						break;

					case token_plus:
						real_add(&r, &a->value, &b->value);
						break;

					case token_minus:
						real_sub(&r, &a->value, &b->value);
						break;

					case token_div:
						real_div(&r, &a->value, &b->value);
						break;

					/* TODO functions here */

					default: /* Comparisions */
						/* FIXME not really working */
						cmp = real_compare(&a->value, &b->value);
						switch (tok->type) {
							case token_lt:
								cmp = (cmp < 0);
								break;

							case token_lteq:
								cmp = (cmp <= 0);
								break;

							case token_gt:
								cmp = (cmp > 0);
								break;

							case token_gteq:
								cmp = (cmp >= 0);
								break;

							case token_eq:
								cmp = (cmp == 0);
								break;
						}

						memcpy(&r, cmp ? &rone : &rzero, sizeof(r));
						break;
				}

				memcpy(&a->value, &r, sizeof(r));
				ufree(b);
			}
			ufree(tok);
		}
	}

	memcpy(o, &rpn_stack->value, sizeof(*o));
	ufree(rpn_stack);
	rpn_stack = NULL;

	/* All related tokens has been consumed and freed */
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
				vga_puts(token_curr->str);
				break;

			case token_real:
			case token_var:
			case token_negative:
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

	intr_assertNotNull(token_curr);

	if (token_curr->type == token_str) {
		prompt = token_curr->str;

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
	list_push(&for_stack, f);
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

			if (real_compare(&f->limit, &f->iter->val) <= 0) {
				list_pop(&for_stack, f);
				ufree(f);
			}
			else {
				jump.line = f->line;
				jump.jump = 1;
			}
			return;
		}
		prev = f;
		f = f->next;
	}

	intr_die(-EINVAL);
}

static void intr_goto(void)
{
	jump.line = intr_expectInteger();
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

	if (condition) {
		jump.line = intr_expectInteger();
		jump.jump = 1;
		return;
	}

	token_curr = token_curr->next;
	if (token_curr != NULL) {
		intr_expect(token_else);

		token_curr = token_curr->next;

		jump.line = intr_expectInteger();
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

	jump.line = intr_expectInteger();
	jump.jump = 1;

	new = intr_malloc(sizeof(*new));
	new->line = line_curr;

	list_push(&gosub_stack, new);

	token_curr = token_curr->next;
	intr_expect(token_none);
}

static void intr_return(void)
{
	struct gosub_elem *elem = gosub_stack;

	intr_expect(token_none);

	intr_assertNotNull(elem);

	jump.line = elem->line;
	jump.jump = 1;

	gosub_stack = elem->next;
	ufree(elem);
}

static void intr_clear(void)
{
	intr_expect(token_none);

	vga_clear();
}

void intr_clean(int8_t hard)
{
	list_ufree(&gosub_stack);
	list_ufree(&for_stack);

	jump.jump = 0;

	if (hard) {
		list_ufree(&variables);
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
		list_ufree(&tstr);
		intr_die(-EINVAL);
	}

	token_curr = tstr->next;

	entry[tstr->type]();

	list_ufree(&tstr);
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

		curr = curr->next;

		if (jump.jump) {
			jump.jump = 0;
			curr = start;
			while ((curr != NULL) && (curr->number != jump.line)) {
				curr = curr->next;
			}

			intr_assertNotNull(curr);
		}
	}

	intr_clean(0);
}
