/* FellaPC Firmaware
 * GPIO
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#include <stdint.h>

static uint8_t * const pio_ora = (void *)0x0a011;
static uint8_t * const pio_orb = (void *)0x0a010;
static uint8_t * const pio_ddra = (void *)0x0a013;
static uint8_t * const pio_ddrb = (void *)0x0a012;

void gpio_dir(uint8_t port, uint8_t dir, uint8_t mask)
{

}

void gpio_set(uint8_t port, uint8_t val, uint8_t mask);
{

}

uint8_t gpio_get(uint8_t port)
{

}
