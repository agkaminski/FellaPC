/* FellaPC Firmaware
 * VIA 6522
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#include "via.h"

static struct {
	volatile uint8_t orb;
	volatile uint8_t ora;
	volatile uint8_t ddrb;
	volatile uint8_t ddra;
	volatile uint8_t t1cl;
	volatile uint8_t t1ch;
	volatile uint8_t t1ll;
	volatile uint8_t t1lh;
	volatile uint8_t t2cl;
	volatile uint8_t t2ch;
	volatile uint8_t sr;
	volatile uint8_t acr;
	volatile uint8_t pcr;
	volatile uint8_t ifr;
	volatile uint8_t ier;
} * const via = (void *)0xa010;

void via_setDirA(uint8_t dir, uint8_t mask)
{
	via->ddra = (via->ddra & ~mask) | (dir & mask);
}

void via_setPortA(uint8_t port, uint8_t mask)
{
	via->ora = (via->ora & ~mask) | (port & mask);
}

uint8_t via_getPortA(void)
{
	return via->ora & (via->ddra);
}

void via_setPinA(uint8_t pin, uint8_t val)
{
	if (val) {
		via->ora |= (1 << pin);
	}
	else {
		via->ora &= ~(1 << pin);
	}
}

void via_setDirB(uint8_t dir, uint8_t mask)
{
	via->ddrb = (via->ddrb & ~mask) | (dir & mask);
}

void via_setPortB(uint8_t port, uint8_t mask)
{
	via->orb = (via->orb & ~mask) | (port & mask);
}

uint8_t via_getPortB(void)
{
	return via->orb & (via->ddrb);
}

void via_setPinB(uint8_t pin, uint8_t val)
{
	if (val) {
		via->orb |= (1 << pin);
	}
	else {
		via->orb &= ~(1 << pin);
	}
}
