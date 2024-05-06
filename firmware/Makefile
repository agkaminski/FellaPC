GOAL = firmware

CPU = 6502
TARGET = $(addsuffix .bin, ${GOAL})
GHASH = "$(shell git describe --always --dirty)"
OBJ = main.o dram.o interrupt.o vga.o keyboard.o keyboard_low.o tty.o \
cmd.o real.o real_asm.o ualloc/ualloc_init.o ualloc/umalloc.o ualloc/ufree.o \
token.o interpreter.o list.o helper.o

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
