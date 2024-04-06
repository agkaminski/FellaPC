CPU = 6502

.PHONY: clean

all: fellapc.lib

fellapc.lib: none.lib crt0.S
	@ca65 --cpu ${CPU} crt0.S
	@cp none.lib fellapc.lib
	@ar65 a fellapc.lib crt0.o

clean:
	@rm -f fellapc.lib *.o
