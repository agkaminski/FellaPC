/* FellaPC Firmaware
 * BASIC interpreter
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include <stdint.h>
#include "token.h"

int8_t interpreter(struct token *tstr);

#endif