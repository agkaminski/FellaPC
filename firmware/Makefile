GOAL = firmware

CPU = 6502
TARGET = $(addsuffix .bin, ${GOAL})
GHASH = "$(shell git describe --always --dirty)"
OBJ = main.o dram.o interrupt.o vga_clr.o vga.o keyboard.o vga_scroll.o tty.o via.o \
ualloc/ualloc.o cmd.o

.PHONY: clean
.PRECIOUS: %.asm
MAKEFLAGS += --no-builtin-rules

all: ${TARGET}
	@grep -A11 "Segment list:" ${GOAL}.map

fellapc.lib:
	@cd bsp && $(MAKE) all --no-print-directory

${TARGET}: ${OBJ} fellapc.lib
	@ld65 -C bsp/fellapc.cfg -m ${GOAL}.map -o "$@" ${OBJ} bsp/fellapc.lib

%.asm: %.c
	@cc65 -I bsp -o "$@" -t none -O --cpu ${CPU} -DVERSION='${GHASH}' "$<"

%.o: %.s
	@ca65 --cpu ${CPU} "$<" -o "$@"

%.o: %.asm
	@ca65 --cpu ${CPU} "$<" -o "$@"

clean:
	@rm -f *.o *.asm *.map *.bin
	@cd bsp && $(MAKE) clean --no-print-directory
