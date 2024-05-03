/* FellaPC Firmaware
 * Tokenizer
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#ifndef TOKEN_H_
#define TOKEN_H_

enum token_type {
	/* Data */
	token_var,
	token_real,

	/* Operators */
	token_plus,
	token_minus,
	token_mul,
	token_div,
	token_mod,
	token_lbrace,
	token_rbrace,
	token_eq,
	token_lt,
	token_lteq,
	token_gt,
	token_gteq,

	/* Keywords */
	token_print,
	token_input,
	token_for,
	token_to,
	token_next,
	token_goto,
	token_if,
	token_then,
	token_dim,
	token_gosub,
	token_return,
	token_clear,

	/* Functions */
	token_abs,
	token_and,
	token_atn,
	token_cos,
	token_exp,
	token_int,
	token_log,
	token_not,
	token_or,
	token_rnd,
	token_sgn,
	token_sin,
	token_sqr,
	token_tan,
	token_len,
	token_chr,
	token_mid,
	token_left,
	token_right,
	token_asc
};

struct token {
	struct token *next;
	enum token_type type;
	char *value;
};

struct token *token_tokenize(const char *line);

#endif
