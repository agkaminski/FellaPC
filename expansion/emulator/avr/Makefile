ISP ?= usbasp
#ISP ?= avrisp2

all: emuprom.c
	avr-gcc -DF_CPU=14745600UL -O2 -mmcu=atmega8 -Wall emuprom.c uart/uart.c md5/md5.c -o bin/emuprom
	avr-objcopy -Oihex bin/emuprom bin/emuprom.hex
	size -A -d bin/emuprom

fuse:
	avrdude -c${ISP} -pm8 -U lfuse:w:0xef:m -U hfuse:w:0xd9:m

install:
	avrdude -c${ISP} -pm8 -U flash:w:bin/emuprom.hex:i

clean:
	rm -f bin/*
