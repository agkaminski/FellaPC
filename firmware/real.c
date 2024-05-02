/* FellaPC Firmaware
 * Real numbers
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#include <string.h>
#include <stdlib.h>

#include "real.h"

uint8_t _real_acc[PRECISION];

extern uint8_t _real_bcdAdd(const uint8_t *a);
extern void _real_bcdSub(const uint8_t *a);

/* Macro is cheaper than function, what a crazy compiler it is */
#define SWAP(a, b) \
do { \
	const real *_swap = a; \
	a = b; \
	b = _swap; \
} while (0)

static void real_shiftLeft(real *r)
{
	uint8_t i;
	uint8_t rem = 0, t;

	for (i = 0; i < sizeof(r->m); ++i) {
		t = (r->m[i] << 4) | rem;
		rem = r->m[i] >> 4;
		r->m[i] = t;
	}
}

static void real_shiftRight(real *r)
{
	int8_t i;
	uint8_t rem = 0, t;

	for (i = sizeof(r->m) - 1; i >= 0; --i) {
		t = (r->m[i] >> 4) | (rem << 4);
		rem = r->m[i] & 0xf;
		r->m[i] = t;
	}
}

static int8_t real_cmp(const real *a, const real *b)
{
	int8_t i;

	for (i = sizeof(a->m) - 1; i >= 0 ; --i) {
		if (a->m[i] == b->m[i]) {
			continue;
		}

		if (a->m[i] > b->m[i]) {
			return 1;
		}
		else {
			return -1;
		}
	}

	return 0;
}

void real_normalize(real *r)
{
	uint8_t i;
	int8_t zero = 1;

	for (i = 0; i < sizeof(r->m); ++i) {
		if (r->m[i] != 0) {
			zero = 0;
			break;
		}
	}

	if (zero) {
		r->e = 0;
		r->s = 1;
	}

	while ((r->m[(PRECISION / 2) - 1] & 0xf0) == 0) {
		if (r->e == INT8_MIN) {
			break;
		}

		real_shiftLeft(r);
		--r->e;
	}
}

void real_rtoa(char *buff, const real *r)
{
	int8_t zero = 1;
	uint8_t i, pos = 0, sign = 0, dot = 1;
	int16_t e = r->e;

	if (r->s < 0) {
		buff[pos++] = '-';
		sign = 1;
	}

	for (i = 0; i < PRECISION; ++i) {
		uint8_t dig = r->m[(PRECISION - i - 1) >> 1];
		if (i & 1) {
			dig &= 0xf;
		}
		else {
			dig >>= 4;
		}

		if ((!dig) && zero) {
			--e;
			continue;
		}
		zero = 0;

		buff[pos++] = '0' + dig;
	}

	if (zero) {
		buff[0] = '0';
		buff[1] = '\0';
		return;
	}

	buff[pos] = '\0';

	if ((e > 0) && (e < PRECISION)) {
		dot += e;
		e = 0;
	}

	if (dot < (pos - sign)) {
		memmove(buff + sign + dot + 1, buff + sign + dot, pos - dot + 1);
		buff[dot + sign] = '.';
		++pos;

		/* Trim trailing zeroes */
		while (buff[pos - 1] == '0') {
			buff[pos - 1] = '\0';
			--pos;
		}

		if (buff[pos - 1] == '.') {
			buff[pos - 1] = '\0';
			--pos;
		}
	}

	if (e != 0) {
		buff[pos++] = 'E';
		itoa(e, &buff[pos], 10);
	}
}

int8_t real_ator(const char *buff, real *r)
{
	/* TODO */
	(void)buff;
	(void)r;
	return -1;
}

int8_t real_add(real *o, const real *a, const real *b)
{
	uint8_t carry;

	if (a->s != b->s) {
		real t;
		memcpy(&t, b, sizeof(t));
		t.s = (t.s < 0) ? 1 : -1;
		return real_sub(o, a, &t);
	}

	if (b->e < a->e) {
		SWAP(a, b);
	}

	memcpy(o, a, sizeof(*o));

	while (o->e < b->e) {
		real_shiftRight(o);
		++o->e;
	}

	memcpy(_real_acc, o->m, sizeof(o->m));
	carry = _real_bcdAdd(b->m);
	memcpy(o->m, _real_acc, sizeof(o->m));

	if (carry) {
		if (o->e == INT8_MAX) {
			return -1;
		}

		real_shiftRight(o);
		++o->e;
		o->m[sizeof(o->m) - 1] |= 0x10;
	}

	return 0;
}

int8_t real_sub(real *o, const real *a, const real *b)
{
	int8_t cmp;

	if (a->s != b->s) {
		real t;
		memcpy(&t, b, sizeof(t));
		t.s = (t.s < 0) ? 1 : -1;
		return real_add(o, a, &t);
	}

	if (b->e < a->e) {
		SWAP(a, b);
	}

	*o = *a;

	while (o->e < b->e) {
		real_shiftRight(o);
		++o->e;
	}

	cmp = real_cmp(a, b);
	switch (cmp) {
		case 1:
			memcpy(_real_acc, o->m, sizeof(_real_acc));
			_real_bcdSub(b->m);
			break;

		case -1:
			memcpy(_real_acc, b->m, sizeof(_real_acc));
			_real_bcdSub(o->m);
			break;

		default:
			memset(o->m, 0, sizeof(o->m));
			o->e = 0;
			o->s = 1;
			return 0;
	}

	memcpy(o->m, _real_acc, sizeof(o->m));

	real_normalize(o);

	return 0;
}

int8_t real_mul(real *o, const real *a, const real *b)
{
	/* TODO */
	(void)o;
	(void)a;
	(void)b;
	return -1;
}

int8_t real_div(real *o, const real *a, const real *b)
{
	/* TODO */
	(void)o;
	(void)a;
	(void)b;
	return -1;
}
