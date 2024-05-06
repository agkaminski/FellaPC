/* FellaPC Firmaware
 * Tokenizer
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "token.h"
#include "ualloc/ualloc.h"
#include "real.h"

static const struct {
	const char *str;
	enum token_type type;
} tokstr[] = {
	{ "print", token_print },
	{ "input", token_input },
	{ "for", token_for },
	{ "to", token_to },
	{ "step", token_step },
	{ "next", token_next },
	{ "goto", token_goto },
	{ "if", token_if },
	{ "then", token_then },
	{ "else", token_else },
	{ "dim", token_dim },
	{ "gosub", token_gosub },
	{ "return", token_return },
	{ "clear", token_clear },
	{ "abs", token_abs },
	{ "and", token_and },
	{ "atn", token_atn },
	{ "cos", token_cos },
	{ "exp", token_exp },
	{ "int", token_int },
	{ "log", token_log },
	{ "not", token_not },
	{ "or", token_or },
	{ "rnd", token_rnd },
	{ "sgn", token_sgn },
	{ "sin", token_sin },
	{ "sqr", token_sqr },
	{ "tan", token_tan },
	{ "len", token_len },
	{ "chr$", token_chr },
	{ "mid$", token_mid },
	{ "left$", token_left },
	{ "right$", token_right },
	{ "asc", token_asc },
	{ "peek", token_peek },
	{ "poke", token_poke }
};

void token_push(struct token **queue, struct token *t)
{
	t->prev = NULL;
	t->next = (*queue);
	if ((*queue) != NULL) {
		(*queue)->prev = t;
	}
	(*queue) = t;
}

void token_append(struct token **queue, struct token *t)
{
	struct token *head = *queue;

	if ((*queue) == NULL) {
		token_push(queue, t);
	}
	else {
		while (head->next != NULL) {
			head = head->next;
		}

		head->next = t;
		t->prev = head;
		t->next = NULL;
	}
}

void token_pop(struct token **queue, struct token *t)
{
	if (t == (*queue)) {
		(*queue) = t->next;
	}

	if (t->next != NULL) {
		t->next->prev = t->prev;
	}

	if (t->prev != NULL) {
		t->prev->next = t->next;
	}

	t->next = NULL;
	t->prev = NULL;
}

void token_free(struct token **first)
{
	struct token *victim;

	while ((*first) != NULL) {
		victim = (*first);
		(*first) = (*first)->next;
		ufree(victim->str);
		ufree(victim);
	}
}

int8_t token_tokenize(struct token **tstr, const char *line)
{
	uint8_t pos = 0, start, isreal, isstr;
	struct token *first = NULL, *curr, *prev = NULL;

	while (line[pos] != '\0') {
		while ((line[pos] == ' ') || (line[pos] == '\t')) {
			++pos;
		}

		isreal = (isdigit(line[pos])) || (line[pos] == '.');

		if (line[pos] == '\"') {
			isstr = 1;
			++pos;
		}
		else {
			isstr = 0;
		}

		start = pos;

		/* Cut the token */

		while (line[pos] != '\0') {
			if (isreal) {
				if (!isdigit(line[pos]) && (line[pos] != '.')) {
					if (isalpha(line[pos])) {
						return -EINVAL;
					}
					break;
				}
			}
			else if (isstr) {
				if (line[pos] == '\"') {
					break;
				}
			}
			else if (!isalnum(line[pos]) && (line[pos] != '$')) {
				break;
			}

			++pos;
		}

		if (isstr && (line[pos] != '\"')) {
			return -EINVAL;
		}

		curr = umalloc(sizeof(*curr));
		if (curr == NULL) {
			token_free(&first);
			return -ENOMEM;
		}
		curr->str = NULL;
		curr->next = NULL;
		curr->prev = NULL;

		if (start != pos) {
			uint8_t tpos = 0;

			curr->str = umalloc(pos - start + 1);
			if (curr->str == NULL) {
				ufree(curr);
				token_free(&first);
				return -ENOMEM;
			}
			memcpy(curr->str, line + start, pos - start);
			curr->str[pos - start] = '\0';

			if (isreal) {
				const char *ret;

				curr->type = token_real;
				ret = real_ator(curr->str, &curr->value);
				ufree(curr->str);
				curr->str = NULL;

				if (ret == NULL) {
					ufree(curr);
					token_free(&first);
					return -EINVAL;
				}
			}
			else if (isstr) {
				curr->type = token_str;
				++pos; /* Eat closing " */
			}
			else {
				uint8_t i, found = 0;
				for (i = 0; i < sizeof(tokstr) / sizeof(*tokstr); ++i) {
					if (strcasecmp(tokstr[i].str, curr->str) == 0) {
						curr->type = tokstr[i].type;
						found = 1;
						break;
					}
				}

				if (!found) {
					curr->type = token_var;
				}
				else {
					ufree(curr->str);
					curr->str = NULL;
				}
			}
		}
		else {
			/* Operators. Just convert to token and hope for the best.
			 * Invalid token should be get by the interpreter, so
			 * theoretically it's 100% ok */
			if ((line[pos] < '%') || (line[pos] > '>')) {
				ufree(curr);
				token_free(&first);
				return -EINVAL;
			}
			curr->type = (enum token_type)line[pos];
			if (((curr->type == token_lt) || (curr->type == token_gt)) && (line[pos + 1] == '=')) {
				curr->type += 10;
				++pos;
			}

			if ((curr->type == token_minus) &&
					((prev == NULL) || ((prev->type != token_var) && (prev->type != token_real)))) {
				curr->type = token_negative;
			}
			++pos;
		}

		token_append(&first, curr);
		prev = curr;
	}

	*tstr = first;

	return 0;
}
