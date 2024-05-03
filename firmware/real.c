/* FellaPC Firmaware
 * Real numbers
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "real.h"

extern uint8_t _real_bcdAdd(uint8_t *a, uint8_t *b);
extern void _real_bcdSub(uint8_t *a, uint8_t *b);

static const real rzero = { { 0, 0, 0, 0, 0 }, 0, 1 };

/* Macro is cheaper than function, what a crazy compiler it is */
#define SWAP(a, b) \
do { \
	const real *_swap = a; \
	a = b; \
	b = _swap; \
} while (0)

#define LOW(x) ((x) & 0xf)
#define HIGH(x) ((x) >> 4)

static int8_t real_isZero(const real *r)
{
	uint8_t i;

	for (i = 0; i < sizeof(r->m); ++i) {
		if (r->m[i] != 0) {
			return 0;
		}
	}

	return 1;
}

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
	if (real_isZero(r)) {
		r->e = 0;
		r->s = 1;
		return;
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
	uint8_t i, pos = 0, sign = 0, dot = 1;
	int16_t e;
	real t;

	memcpy(&t, r, sizeof(t));

	if (real_isZero(&t)) {
		buff[0] = '0';
		buff[1] = '\0';
		return;
	}

	if (t.s < 0) {
		buff[pos++] = '-';
		sign = 1;
	}

	if ((t.e < 0) && (t.e > -PRECISION)) {
		while (((t.m[0] & 0xf) == 0) && (t.e < 0)) {
			real_shiftRight(&t);
			++t.e;
		}
	}
	e = t.e;

	for (i = 0; i < PRECISION; ++i) {
		uint8_t dig = t.m[(PRECISION - i - 1) >> 1];
		dig = (i & 1) ? LOW(dig) : HIGH(dig);
		buff[pos++] = '0' + dig;
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

		/* Trim leading zeroes */
		for (i = sign; ; ++i) {
			if ((buff[i] == '0') && (buff[i + 1] != '.')) {
				continue;
			}
			break;
		}
		if (i != sign) {
			memmove(buff + sign, buff + sign + i, pos - sign - i + 1);
		}
		e -= i;
	}

	if (e != 0) {
		buff[pos++] = 'E';
		itoa(e, &buff[pos], 10);
	}
}

const char *real_ator(const char *buff, real *r)
{
	int8_t dot = 0, mpos;
	uint8_t pos = 0;

	memset(r->m, 0, sizeof(r->m));
	r->s = 1;
	r->e = -1;

	if (buff[pos] == '-') {
		r->s = -1;
		++pos;
	}

	while (buff[pos] == '0') {
		++pos;
	}

	if (buff[pos] == '.') {
		++pos;
		while (buff[pos] == '0') {
			++pos;
			--r->e;
		}
		dot = 1;
	}

	for (mpos = PRECISION - 1; mpos >= 0; --mpos) {
		char digit;
		if (!isdigit(buff[pos])) {
			if ((buff[pos] == '.') && !dot) {
				dot = 1;
				++pos;
				if (!isdigit(buff[pos])) {
					break;
				}
			}
			else {
				break;
			}
		}
		digit = buff[pos++] - '0';
		r->m[mpos >> 1] |= (mpos & 1) ? (digit << 4) : digit;

		if (!dot) {
			++r->e;
		}
	}

	if ((buff[pos] == 'E') || (buff[pos] == 'e')) {
		r->e += atoi(&buff[++pos]);
		 while (isdigit(buff[pos])) {
			++pos;
		}
	}

	return &buff[pos];
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

	carry = _real_bcdAdd(o->m, b->m);

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
	int8_t signswap = 0;
	uint8_t temp[PRECISION / 2];

	if (a->s != b->s) {
		real t;
		memcpy(&t, b, sizeof(t));
		t.s = (t.s < 0) ? 1 : -1;
		return real_add(o, a, &t);
	}

	if (b->e < a->e) {
		SWAP(a, b);
		signswap = 1;
	}

	memcpy(o, a, sizeof(*o));

	if (signswap) {
		o->s = (o->s < 0) ? 1 : -1;
	}

	while (o->e < b->e) {
		real_shiftRight(o);
		++o->e;
	}

	cmp = real_cmp(o, b);
	switch (cmp) {
		case 1:
			_real_bcdSub(o->m, b->m);
			break;

		case -1:
			memcpy(temp, b->m, sizeof(temp));
			_real_bcdSub(temp, o->m);
			memcpy(o->m, temp, sizeof(o->m));
			o->s = (o->s < 0) ? 1 : -1;
			break;

		default:
			memcpy(o, &rzero, sizeof(*o));
			return 0;
	}

	real_normalize(o);

	return 0;
}

int8_t real_mul(real *o, const real *a, const real *b)
{
	real t, acc;
	uint8_t i;

	memcpy(o, &rzero, sizeof(*o));
	memcpy(&acc, &rzero, sizeof(t));
	memcpy(&t, a, sizeof(t));

	t.e -= 9;

	for (i = 0; i < PRECISION; ++i) {
		uint8_t cnt = b->m[i >> 1];
		cnt = (i & 1) ? HIGH(cnt) : LOW(cnt);

		while (cnt--) {
			if (real_add(o, &acc, &t) < 0) {
				return -1;
			}
			memcpy(&acc, o, sizeof(acc));
		}
		++t.e;
	}

	o->e += b->e;
	o->s = (a->s == b->s) ? 1 : -1;

	return 0;
}

int8_t real_div(real *o, const real *a, const real *b)
{
	/* TODO */
	(void)o;
	(void)a;
	(void)b;
	return -1;
}
