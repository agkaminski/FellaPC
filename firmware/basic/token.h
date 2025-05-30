/* FellaPC Firmaware
 * Tokenizer
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#ifndef TOKEN_H_
#define TOKEN_H_

#include "real.h"

#define TOKEN_FUNCTION_START 128
#define TOKEN_OPERATOR_START 40

enum token_type {
	/* Beginning tokens */

	/* Data */
	token_var = 0,

	/* Keywords */
	token_print,
	token_input,
	token_for,
	token_next,
	token_goto,
	token_if,
	token_gosub,
	token_return,
	token_clear,
	token_end,

	/* Following tokens */

	/* Data */
	token_real,
	token_str,

	/* Keywords */
	token_to,
	token_step,
	token_then,
	token_else,

	/* Operators */
	token_lpara = '(',
	token_rpara = ')',
	token_mul = '*',
	token_plus = '+',
	token_coma = ',',
	token_minus = '-',
	token_div = '/',
	token_semicol = ';',
	token_lt = '<',
	token_eq = '=',
	token_gt = '>',
	token_lteq = token_lt + 10,
	token_gteq = token_gt + 10,
	token_negative,
	token_and,
	token_not,
	token_or,

	/* Functions */
	token_abs = TOKEN_FUNCTION_START,
	token_fre,
	token_int,
	token_rnd,
	token_peek,
	token_poke,

	/* Error */
	token_none
};

struct token {
	struct token *next;
	struct token *prev;
	enum token_type type;
	real value;
	char str[];
};

int8_t token_isValue(enum token_type type);

int8_t token_tokenize(struct token **tstr, const char *line);

#endif
