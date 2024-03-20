GOAL = rom

CPU = 6502
TARGET = $(addsuffix .bin, ${GOAL})
GHASH = "$(shell git describe --always --dirty)"
OBJ = main.o interrupt.o keyboard.o

.PHONY: clean
.PRECIOUS: %.asm
MAKEFLAGS += --no-builtin-rules

all: ${TARGET}

fellapc.lib:
	@cd bsp && $(MAKE) all

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
	@cd bsp && $(MAKE) clean
