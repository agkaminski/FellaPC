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

extern const real rzero;
extern const real rone;

int8_t real_isZero(const real *r);

void real_normalize(real *r);

void real_rtoa(char *buff, const real *r);

/* Super unsafe, but small */
int real_rtoi(const real *r);

const char *real_ator(const char *buff, real *r);

int8_t real_add(real *o, const real *a, const real *b);

int8_t real_sub(real *o, const real *a, const real *b);

int8_t real_mul(real *o, const real *a, const real *b);

int8_t real_div(real *o, const real *a, const real *b);

#endif
