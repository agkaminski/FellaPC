/* FellaPC Firmaware
 * Tokenizer
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "token.h"
#include "ualloc/ualloc.h"

static void token_free(struct token *first)
{
	struct token *victim;

	while (first != NULL) {
		victim = first;
		first = first->next;
		ufree(victim->value);
		ufree(victim);
	}
}

static struct {
	const char *str;
	enum token_type type;
} tokstr[] = {
	{ "print", token_print },
	{ "input", token_input },
	{ "for", token_for },
	{ "to", token_to },
	{ "next", token_next },
	{ "goto", token_goto },
	{ "if", token_if },
	{ "then", token_then },
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
	{ "asc", token_asc }
};

struct token *token_tokenize(const char *line)
{
	uint8_t pos = 0, start, isreal;
	struct token *first = NULL, *prev, *curr;

	while (line[pos] != '\0') {
		curr = umalloc(sizeof(*curr));
		if (curr == NULL) {
			token_free(first);
			return NULL;
		}
		curr->value = NULL;
		curr->next = NULL;

		while ((line[pos] == ' ') || (line[pos] == '\t')) {
			++pos;
		}

		start = pos;

		if (isdigit(line[pos])) {
			isreal = 1;
		}

		/* Cut the token */

		while (isalnum(line[pos]) || (line[pos] == '$')) {
			if (isreal && !isdigit(line[pos])) {
				token_free(first);
				return NULL;
			}

			++pos;
		}

		if (start != pos) {
			uint8_t tpos = 0;

			curr->value = umalloc(pos - start + 1);
			if (curr->value == NULL) {
				token_free(first);
				return NULL;
			}
			memcpy(curr->value, line + start, pos - start);
			curr->value[pos - start] = '\0';

			if (isreal) {
				curr->type = token_real;
			}
			else {
				uint8_t i, found = 0;
				for (i = 0; i < sizeof(tokstr) / sizeof(*tokstr); ++i) {
					if (strcasecmp(tokstr->str, curr->value) == 0) {
						curr->type = tokstr->type;
						found = 1;
						break;
					}
				}

				if (!found) {
					curr->type = token_var;
				}
				else {
					ufree(curr->value);
					curr->value = NULL;
				}
			}
		}
		else {
			switch (line[pos]) {
				case '+':
					curr->type = token_plus;
					break;

				case '-':
					curr->type = token_minus;
					break;

				case '*':
					curr->type = token_mul;
					break;

				case '/':
					curr->type = token_div;
					break;

				case '%':
					curr->type = token_mod;
					break;

				case '(':
					curr->type = token_lbrace;
					break;

				case ')':
					curr->type = token_rbrace;
					break;

				case '=':
					curr->type = token_eq;
					break;

				case '<':
					curr->type = token_lt;
					if (line[pos + 1] == '=') {
						curr->type = token_lteq;
						++pos;
					}
					break;

				case '>':
					curr->type = token_gt;
					if (line[pos + 1] == '=') {
						curr->type = token_gteq;
						++pos;
					}
					break;

				default:
					token_free(first);
					return NULL;
			}
			++pos;
		}

		if (first == NULL) {
			first = curr;
		}
		else {
			prev->next = curr;
		}
		prev = curr;
	}

	return first;
}