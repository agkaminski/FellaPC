/* FellaPC Firmaware
 * Real numbers
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "real.h"
#include "system.h"

extern uint8_t _real_bcdAdd(uint8_t *a, uint8_t *b);
extern void _real_bcdSub(uint8_t *a, uint8_t *b);

const real rzero = { { 0, 0, 0, 0, 0 }, 0, 1 };
const real rone = { { 0, 0, 0, 0, 0x10 }, 0, 1 };

/* Macro is cheaper than function, what a crazy compiler it is */
#define SWAP(a, b) \
do { \
	const real *_swap = a; \
	a = b; \
	b = _swap; \
} while (0)

#define LOW(x) ((x) & 0xf)
#define HIGH(x) ((x) >> 4)

void real_shiftLeft(real *r);
void real_shiftRight(real *r);
int8_t real_cmp(const real *a, const real *b);

void real_rtoa(char *buff, const real *r)
{
	uint8_t i, pos = 0, sign = 0, dot = 1;
	int16_t e;
	real t;

	real_copy(&t, r);

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
		e -= i - sign;
	}

	if (e != 0) {
		buff[pos++] = 'E';
		itoa(e, &buff[pos], 10);
	}
}

int real_rtoi(const real *r)
{
	char buff[PRECISION + 1 + 1 + 5 + 1]; /* sign, dot, exponent, null term */
	real_rtoa(buff, r);
	return atoi(buff);
}

void real_itor(real *r, int i)
{
	char buff[20];
	itoa(i, buff, 10);
	real_ator(buff, r);
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

	while (isdigit(buff[pos])) {
		if (!dot) {
			++r->e;
		}
		++pos;
	}

	if ((buff[pos] == 'E') || (buff[pos] == 'e')) {
		int e = atoi(&buff[++pos]) + r->e;
		if (e > INT8_MAX) {
			return NULL;
		}
		r->e = e;
		if (buff[pos] == '-') {
			++pos;
		}
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
		real_copy(&t, b);
		t.s = -t.s;
		return real_sub(o, a, &t);
	}

	if (b->e < a->e) {
		SWAP(a, b);
	}

	real_copy(o, a);

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

int8_t real_add2(real *o, const real *b)
{
	real t;
	int8_t ret = real_add(&t, o, b);
	real_copy(o, &t);
	return ret;
}

int8_t real_sub(real *o, const real *a, const real *b)
{
	int8_t cmp;
	int8_t signswap = 0;
	real temp;

	if (a->s != b->s) {
		real t;
		real_copy(&t, b);
		t.s = -t.s;
		return real_add(o, a, &t);
	}

	if (b->e < a->e) {
		SWAP(a, b);
		signswap = 1;
	}

	real_copy(o, a);

	if (signswap) {
		o->s = -o->s;
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
			real_copy(&temp, b);
			temp.e = o->e;
			temp.s = -o->s;
			_real_bcdSub(temp.m, o->m);
			real_copy(o, &temp);
			break;

		default:
			real_setZero(o);
			return 0;
	}

	real_normalize(o);

	return 0;
}

int8_t real_sub2(real *o, const real *b)
{
	real t;
	int8_t ret = real_sub(&t, o, b);
	real_copy(o, &t);
	return ret;
}

int8_t real_mul2(real *a, const real *b)
{
	real acc;
	uint8_t i;

	real_setZero(&acc);

	a->e -= 9;

	for (i = 0; i < PRECISION; ++i) {
		uint8_t cnt = b->m[i >> 1];
		cnt = (i & 1) ? HIGH(cnt) : LOW(cnt);

		while (cnt--) {
			real_add2(&acc, a);
		}
		++a->e;
	}

	acc.e += b->e;
	acc.s = (a->s == b->s) ? 1 : -1;

	real_copy(a, &acc);

	return 0;
}

int8_t real_div2(real *a, const real *b)
{
	real t, acc;
	int8_t i, sign = a->s;

	if (real_isZero(b)) {
		return -1;
	}

	real_copy(&t, a);
	memset(a->m, 0, sizeof(a->m));

	t.s = b->s;
	t.e = b->e;

	for (i = PRECISION - 1; i > 0; --i) {
		while (1) {
			real_sub(&acc, &t, b);
			if (acc.s != t.s) {
				break;
			}
			a->m[i >> 1] += (i & 1) ? 0x10 : 1;
			real_copy(&t, &acc);
		}
		++t.e;
	}

	a->e -= b->e;
	a->s = (sign == b->s) ? 1 : -1;

	real_normalize(a);

	return 0;
}

int8_t real_compare(const real *a, const real *b)
{
	real x, t;

	real_copy(&x, a);

	if (a->s != b->s) {
		return a->s;
	}

	x.s = b->s;

	real_sub(&t, &x, b);

	if (real_isZero(&t)) {
		return 0;
	}

	if (t.s < 0) {
		return -a->s;
	}

	return a->s;
}

void real_int(real *r)
{
	if (!real_isZero(r)) {
		if (r->s < 0) {
			real_sub2(r, &rone);
		}

		if (r->e < 0) {
			real_setZero(r);
		}
		else {
			for (; r->e < (PRECISION - 1); ++r->e) {
				real_shiftRight(r);
			}
		}

		real_normalize(r);
	}
}

static uint16_t rand16(uint16_t *seed)
{
	unsigned long next = (unsigned long)(*seed) * 1103515243 + 12345;
	next >>= 16;
	*seed ^= (uint16_t)next;
	return next;
}

void real_rand(real *r)
{
	static uint16_t seed = 0;

	if (seed == 0) {
		seed = system_time();
	}

	real_itor(r, rand16(&seed));
	real_add2(r, r);
	real_add2(r, r);

	r->e = -1;
	r->s = 1;
}
