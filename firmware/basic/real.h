/* FellaPC Firmaware
 * Real numbers
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#ifndef REAL_H_
#define REAL_H_

#include <stdint.h>

#define PRECISION 10
#if PRECISION & 1
#error PRECISION has to even
#endif

typedef struct {
	uint8_t m[PRECISION / 2];
	int8_t e;
	int8_t s;
} real;

void real_copy(real *dest, const real *src);

void real_setZero(real *r);

void real_setOne(real *r);

int8_t real_isZero(const real *r);

void real_normalize(real *r);

void real_rtoa(char *buff, const real *r);

/* Super unsafe, but small */
int real_rtoi(const real *r);

void real_itor(real *r, int i);

const char *real_ator(const char *buff, real *r);

void real_add(real *o, const real *a, const real *b);

void real_add2(real *o, const real *b);

void real_sub(real *o, const real *a, const real *b);

void real_sub2(real *o, const real *b);

void real_mul2(real *a, const real *b);

void real_div2(real *a, const real *b);

int8_t real_compare(const real *a, const real *b);

void real_int(real *r);

void real_rand(real *r);

#endif
