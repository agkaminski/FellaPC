# FellaPC

All-in-keyboard 6502 based PC with built-in BASIC interpreter.

Features:

- 6502/65C02/W65C02S CPU @1.5734375 MHz,
- 16 KB of ROM, 32 KB of RAM (SRAM or DRAM),
- 640x480@60Hz VGA adapter (80x60 black and white text mode only),
- 6522 VIA for onboard control, I2C memory and user port,
- 2 UARTs (USB-C and RS-232),
- 69% mechanical keyboard,
- Edge connector expansion port,
- powered via USB-C port.

# Memory map

[Memory map](memmap.md)

# BASIC

FellaPC provides BASIC language subset (partially inspired by C64 BASIC).

## Program editing

Simple line editor is provided, code is entered line by line. To insert a new line or
overwrite existing line type in the line number followed by the code.

Example

```
10 FOR I = 0 TO 10
20 PRINT I
30 NEXT I
```

Let's say we want to change line number 20 to print a square of I insted. To do this
simply type in a new version of this line:

```
20 PRINT I*I
```

The program will be updated:

```
LIST

10 FOR I = 0 TO 10
20 PRINT I
30 NEXT I

Ready
```

## Commands

Following commands are supported:

- PRINT
- INPUT
- FOR
- TO
- STEP
- NEXT
- GOTO
- IF
- ELSE
- GOSUB
- RETURN
- CLEAR
- END

Following functions:

- ABS
- FRE
- INT
- RND
- PEEK
- POKE

And following operators:

( ) * + , - / ; < = > <= >=

Variables are declared on the first use:

```
a = 5
var = 42
```

# Notes

Due to the limited size of the ROM space and poor performance of the CC65 compiler,
no UART and VIA support is present in the FW. It is still possible to use them via
PEEK/POKE instruction or in a custom FW.

Visual glitches are expected on the VGA output when VRAM is being modified. CPU is
too slow and DRAM refresh takes too long to be able to perfom any operation during
VBLANK.

# PCB

<img src="img/top.png">
