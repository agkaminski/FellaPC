/* FellaPC Firmaware
 * BASIC interpreter
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include <stdint.h>
#include "token.h"
#include "cmd.h"

void intr_clean(int8_t hard);

int8_t intr_line(const char *line);

int8_t intr_run(struct line *start);

#endif
