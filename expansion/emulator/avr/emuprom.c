/*
 * PORTD[7..2] - PORTC[1..0]: data
 *
 * PORTB0: Latch Enable high
 * PORTB1: /RESET
 * PORTB2: Latch Enable low
 *
 * PORTB4: /OE
 * PORTB5: /CS
 *
 * PORTC2: /WE
 * PORTC3: Claim by AVR (active low)
 * PORTC4: Claim by 6502 (active low)
 * PORTC5: /IRQ
 *
 * PORTD0: RX
 * PORTD1: TX
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "uart/uart.h"
#include "md5/md5.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

static inline void nop(void)
{
	__asm__ volatile ("\tnop;");
}

#define SET(port, pin) ((port) |= (1 << (pin)))

#define CLEAR(port, pin) ((port) &= ~(1 << (pin)))

#define TOGGLE_HIGH(port, pin) \
	do { \
		nop(); \
		SET(port, pin); \
		nop(); \
		CLEAR(port, pin); \
		nop(); \
	} while (0)

#define TOGGLE_LOW(port, pin) \
	do { \
		nop(); \
		CLEAR(port, pin); \
		nop(); \
		SET(port, pin); \
		nop(); \
	} while (0)

static void claim(void)
{
	/* Reset target */
	CLEAR(PORTB, 1);
	SET(DDRB, 1);

	/* Inactivate control signals */
	SET(PORTC, 2);
	SET(PORTB, 4);
	SET(PORTB, 5);

	/* Claim bus */
	SET(PORTC, 4);
	CLEAR(PORTC, 3);

	/* Set data bus as output */
	DDRD |= 0xfc;
	DDRC |= 0x3;
}

static void release(void)
{
	/* Inactivate control signals */
	SET(PORTC, 2);
	SET(PORTB, 4);
	SET(PORTB, 5);

	/* High-Z data bus */
	DDRD &= ~0xfc;
	DDRC &= ~0x3;

	/* Release bus */
	SET(PORTC, 3);
	CLEAR(PORTC, 4);

	/* Allow-gate /CS and /OE */
	CLEAR(PORTB, 4);
	CLEAR(PORTB, 5);

	/* Release target reset */
	CLEAR(DDRB, 1);
	SET(PORTB, 1);
}

static void setData(uint8_t data)
{
	uint8_t t = PORTC & ~(0x3);
	PORTC = t | (data & 0x3);
	t = PORTD & ~(0xfc);
	PORTD = t | (data & 0xfc);
	nop();
}

static uint8_t getData(void)
{
	uint8_t d = PINC & 0x3;
	d |= PIND & 0xfc;
	return d;
}

static void setAddr(uint16_t addr)
{
	setData(addr & 0xff);
	nop();
	TOGGLE_HIGH(PORTB, 2);
	nop();
	setData(addr >> 8);
	nop();
	TOGGLE_HIGH(PORTB, 0);
	nop();
}

static void write(uint16_t addr, uint8_t data)
{
	setAddr(addr);
	setData(data);
	nop();

	/* Activate /CS */
	CLEAR(PORTB, 5);
	nop();

	/* Write */
	TOGGLE_LOW(PORTC, 2);
	nop();

	/* Inactivate /CS */
	SET(PORTB, 5);
}

static uint8_t read(uint16_t addr)
{
	uint8_t ret = 0xff;

	setAddr(addr);

	/* Data as input */
	DDRC &= ~0x3;
	DDRD &= ~0xfc;
	PORTC |= 0x3;
	PORTD |= 0xfc;

	nop();
	nop();
	nop();

	/* Activate /CS and /OE */
	CLEAR(PORTB, 5);
	CLEAR(PORTB, 4);

	nop();
	nop();
	nop();

	ret = getData();

	/* Inactivate /CS and /OE */
	SET(PORTB, 4);
	SET(PORTB, 5);

	nop();

	/* Bus back as output */
	DDRC |= 0x3;
	DDRD |= 0xfc;

	return ret;
}

static int memtest_fail(uint16_t addr, uint8_t expected, uint8_t got)
{
	char buff[40];
	sprintf(buff, "Fail @0x%04x (expected 0x%02x, got 0x%02x)\n",
		addr, expected, got);
	uart_puts(buff);
	return -1;
}

static void fill(uint8_t val)
{
	for (uint16_t addr = 0; addr < (32u * 1024); ++addr)
		write(addr, val);
}

static int test(uint8_t expected)
{
	for (uint16_t addr = 0; addr < (32u * 1024); ++addr) {
		uint8_t readback = read(addr);
		if (readback != expected) {
			return memtest_fail(addr, expected, readback);
		}
	}
	
	return 0;
}

static int memtest(void)
{
	fill(0);
	if (test(0)) return -1;
	fill(0x55);
	if (test(0x55)) return -1;
	fill(0xaa);
	if (test(0xaa)) return -1;
	fill(0xff);
	if (test(0xff)) return -1;

	return 0;
}

static void dump(uint16_t *addr)
{
	char buff[16];
	uint16_t i, j;

	for (i = 0; i < 256 && i + *addr < 32u * 1024; i += 16) {
		sprintf(buff, "%04x: ", i + *addr);
		uart_puts(buff);
		for (j = 0; j < 16; ++j) {
			uint8_t data = read(*addr + i + j);
			sprintf(buff, "%02x%s", data, (j != 15) ? ", " : "");
			uart_puts(buff);
		}
		uart_putc('\n');
	}
	
	*addr += i;
}

static void help(void)
{
	uart_puts("EPROM Emulator\nA.K. 2024\nCommands:\n");
	uart_puts("\t? - this help\n");
	uart_puts("\t! - claim bus, memory edit, assert /RST\n");
	uart_puts("\t. - release bus, deassert /RST\n");
	uart_puts("\t@ - set address mode\n");
	uart_puts("\t# - set data mode\n");
	uart_puts("\t= - dump 256 bytes @addres\n");
	uart_puts("\tt - memory test\n");
	uart_puts("\t+ - echo on\n");
	uart_puts("\t- - echo off\n");
	uart_puts("\tm - calculate md5 sum\n");
	uart_puts("\ts - clear memory (all 0xff)\n");
	uart_puts("\tr - reset target (pulse /RST)\n");
	uart_puts("\tq - send IRQ (pulse /IRQ)\n");
}

static void md5(void)
{
	MD5Context ctx;
	uint8_t buff[64];

	md5Init(&ctx);

	for (uint16_t i = 0; i < 32u * 1024; i += sizeof(buff)) {
		for (uint16_t j = 0; j < sizeof(buff); ++j) {
			buff[j] = read(i + j);
		}
		md5Update(&ctx, buff, sizeof(buff));
	}

	md5Finalize(&ctx);
	
	uart_puts("md5: ");
	for (uint8_t i = 0; i < sizeof(ctx.digest); ++i) {
		sprintf((char *)buff, "%02x", ctx.digest[i]);
		uart_puts((char *)buff);
	}
	uart_putc('\n');
}

static int parse(char c)
{
	static uint16_t addr;
	static uint8_t data;
	static uint8_t cnt;
	static enum { mode_addr, mode_data } mode = mode_data;
	static bool claimed = true;
	static int echoOff = 0;
	uint8_t val;
	int err;

	if ((c >= '0') && (c <= '9')) {
		val = c - '0';
	}
	else if ((c >= 'a') && (c <= 'f')) {
		val = c + 10 - 'a';
	}
	else if ((c >= 'A') && (c <= 'F')) {
		val = c + 10 - 'A';
	}
	else {
		switch (c) {
			case '!':
				claim();
				claimed = true;
				break;

			case '@':
				mode = mode_addr;
				cnt = 0;
				break;

			case '#':
				mode = mode_data;
				cnt = 0;
				break;

			case '.':
				release();
				claimed = false;
				break;

			case '=':
				if (claimed)
					dump(&addr);
				return -1; /* Supress echo */

			case '+':
				echoOff = 0;
				break;

			case '-':
				echoOff = 1;
				break;

			case 's':
				if (!claimed)
					return -1;
				fill(0xff);
				break;
				
			case 'r':
				CLEAR(PORTB, 1);
				SET(DDRB, 1);
				_delay_ms(100);
				CLEAR(DDRB, 1);
				SET(PORTB, 1);
				break;

			case 'q':
				CLEAR(PORTC, 5);
				SET(DDRC, 5);
				nop();
				CLEAR(DDRC, 5);
				break;

			case 'm':
				if (claimed)
					md5();
				return -1; /* Supress echo */

			case '?':
				help();
				return -1; /* Supress echo */

			case 't':
				err = claimed ? memtest() : -1;
				if (!err) {
					uart_puts("Memory OK\n");
				}
				return -1; /* Supress echo */

			default:
				return -1;
		}

		return echoOff;
	}

	if (mode == mode_addr) {
		addr = (addr << 4) | val;
	}
	else {
		data = (data << 4) | val;
		if (++cnt > 1) {
			if (claimed)
				write(addr, data);
			addr++;
			cnt = 0;
		}
	}

	return echoOff;
}

int main(void)
{
	DDRB = (1 << 5) | (1 << 4) | (1 << 2) | 1;
	DDRC = (1 << 4) | (1 << 3) | (1 << 2);
	DDRD = (1 << 1);

	PORTB = (1 << 5) | (1 << 4) | (1 << 2) | 1;
	PORTC = (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2);
	PORTD = 0;
	
	claim();

	uart_init(7); /* 115200 */
	sei();
	
	fill(0xff);
	help();

	while (1) {
		int rx;
		do {
			rx = uart_getc();
		} while ((rx & ~0xff) != 0);
		
		if (!parse(rx & 0xff)) {
			uart_putc(rx & 0xff);
		}
	}

	return 0;
}
