/* FellaPC Firmaware
 * VIA 6522
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#ifndef VIA_H_
#define VIA_H_

#include <stdint.h>

void via_setDirA(uint8_t dir, uint8_t mask);

void via_setPortA(uint8_t port, uint8_t mask);

uint8_t via_getPortA(void);

void via_setPinA(uint8_t pin, uint8_t val);

void via_setDirB(uint8_t dir, uint8_t mask);

void via_setPortB(uint8_t port, uint8_t mask);

uint8_t via_getPortB(void);

void via_setPinB(uint8_t pin, uint8_t val);

#endif
