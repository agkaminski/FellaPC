/* FellaPC Firmaware
 * TTY
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#ifndef TTY_H_
#define TTY_H_

void tty_resetCursor(void);

int tty_update(char *cmd);

#endif