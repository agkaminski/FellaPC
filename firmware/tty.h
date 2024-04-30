/* FellaPC Firmaware
 * TTY
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#ifndef TTY_H_
#define TTY_H_

#include <stdint.h>

void tty_resetCursor(void);

int8_t tty_update(char *cmd);

#endif
