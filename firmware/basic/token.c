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
#include "list.h"

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
	{ "fre", token_fre },
	{ "int", token_int },
	{ "not", token_not },
	{ "or", token_or },
	{ "rnd", token_rnd },
	{ "time", token_time },
	{ "sqr", token_sqr },
	{ "peek", token_peek },
	{ "poke", token_poke },
	{ "end", token_end }
};

int8_t token_isValue(enum token_type type)
{
	return ((type == token_var) || (type == token_real));
}

int8_t token_tokenize(struct token **tstr, const char *line)
{
	uint8_t pos = 0, start, isreal, isstr;
	struct token *first = NULL, *curr, *prev = NULL;

	while (line[pos] != '\0') {
		while (isspace(line[pos])) {
			++pos;
		}

		start = pos;

		isreal = (isdigit(line[pos])) || (line[pos] == '.');
		if (!isreal) {
			if (line[pos] == '\"') {
				isstr = 1;
				++pos;
				start = pos;
			}
			else {
				isstr = 0;
			}

			/* Cut the token */

			while (line[pos] != '\0') {
				if (isstr) {
					if (line[pos] == '\"') {
						break;
					}
				}
				else if (!isalnum(line[pos]) && (line[pos] != '$') && (line[pos] != '%')) {
					break;
				}

				++pos;
			}

			if (isstr && (line[pos] != '\"')) {
				return -EINVAL;
			}
		}

		curr = umalloc(sizeof(*curr) + pos - start + 1);
		if (curr == NULL) {
			list_ufree(&first);
			return -ENOMEM;
		}

		list_append(&first, curr);

		if (isreal) {
			const char *ret;
			curr->type = token_real;
			ret = real_ator(&line[pos], &curr->value);
			if (ret == NULL) {
				list_ufree(&first);
				return -EINVAL;
			}

			pos = ret - line;
		}
		else if (start != pos) {
			uint8_t tpos = 0;

			memcpy(curr->str, line + start, pos - start);
			curr->str[pos - start] = '\0';

			if (isstr) {
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
			}
		}
		else {
			/* Operators. Just convert to token and hope for the best.
			 * Invalid token should be get by the interpreter, so
			 * theoretically it's 100% ok */
			if ((line[pos] < '(') || (line[pos] > '>')) {
				list_ufree(&first);
				return -EINVAL;
			}
			curr->type = (enum token_type)line[pos];
			if (((curr->type == token_lt) || (curr->type == token_gt)) && (line[pos + 1] == '=')) {
				curr->type += 10;
				++pos;
			}

			if ((curr->type == token_minus) &&
					((prev == NULL) || !token_isValue(prev->type))) {
				curr->type = token_negative;
			}
			++pos;
		}

		prev = curr;
	}

	*tstr = first;

	return 0;
}
