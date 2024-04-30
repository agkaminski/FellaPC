# Memory map

| Start  | End    | Description      |
|--------|--------|------------------|
| 0x0000 | 0x7FFF | RAM              |
| 0x8000 | 0x8002 | VGA              |
| 0x8003 | 0x8FFF | VGA aliased      |
| 0x9000 | 0x900F | Keyboard         |
| 0x9010 | 0x9FFF | Keyboard aliased |
| 0xA000 | 0xA00F | Reserved         |
| 0xA010 | 0xA01F | User port        |
| 0xA020 | 0xA02F | RS232            |
| 0xA030 | 0xA03F | Invalid          |
| 0xA040 | 0xA04F | USB UART         |
| 0xA050 | 0xAFFF | Invalid          |
| 0xB000 | 0xBFFF | Expansion        |
| 0xC000 | 0xFFFF | ROM              |

# I/O register map

| Address | Peripheral | Register  | Description                  |
|---------|------------|-----------|------------------------------|
| 0x8000  | VGA        | VDR       | VGA data register            |
| 0x8001  | VGA        | VALR      | VGA column address           |
| 0x8002  | VGA        | VAHR      | VGA row address              |
| 0x9000  | Keyboard   | COL0      | Keyboard column #0           |
| 0x9001  | Keyboard   | COL1      | Keyboard column #1           |
| 0x9002  | Keyboard   | COL2      | Keyboard column #2           |
| 0x9003  | Keyboard   | COL3      | Keyboard column #3           |
| 0x9004  | Keyboard   | COL4      | Keyboard column #4           |
| 0x9005  | Keyboard   | COL5      | Keyboard column #5           |
| 0x9006  | Keyboard   | COL6      | Keyboard column #6           |
| 0x9007  | Keyboard   | COL7      | Keyboard column #7           |
| 0x9008  | Keyboard   | COL8      | Keyboard column #8           |
| 0x9009  | Keyboard   | COL9      | Keyboard column #9           |
| 0x900A  | Keyboard   | COL10     | Keyboard column #10          |
| 0x900B  | Keyboard   | COL11     | Keyboard column #11          |
| 0x900C  | Keyboard   | COL12     | Keyboard column #12          |
| 0x900D  | Keyboard   | COL13     | Keyboard column #13          |
| 0x900E  | Keyboard   | COL14     | Keyboard column #14          |
| 0x900F  | Keyboard   | COL15     | Keyboard column #15          |
| 0xA010  | User port  | ORB       | Port B output register       |
| 0xA011  | User port  | ORA       | Port A output register       |
| 0xA012  | User port  | DDRB      | Port B direction register    |
| 0xA013  | User port  | DDRA      | Port A direction register    |
| 0xA014  | User port  | T1(L/C)-L | Timer 1 low latch/counter    |
| 0xA015  | User port  | T1C-H     | Timer 1 high counter         |
| 0xA016  | User port  | T1L-L     | Timer 1 low latch            |
| 0xA017  | User port  | T1L-H     | Timer 1 high latch           |
| 0xA018  | User port  | T2(L/C)-L | Timer 2 low latch/counter    |
| 0xA019  | User port  | T2C-H     | Timer 2 high counter         |
| 0xA01A  | User port  | SR        | Shift register               |
| 0xA01B  | User port  | ACR       | Auxiliary control register   |
| 0xA01C  | User port  | PCR       | Peripheral control register  |
| 0xA01D  | User port  | IFR       | Interrupt flag register      |
| 0xA01E  | User port  | IER       | Interrupt enable register    |
| 0xA01F  | User port  | ORA       | Port A output register       |
| 0xA020  | RS232      | TDR1/RDR1 | UART 1 data register         |
| 0xA021  | RS232      | RST1/SR1  | UART 1 reset/status register |
| 0xA022  | RS232      | CMDR1     | UART 1 command register      |
| 0xA023  | RS232      | CR1       | UART 1 control register      |
| 0xA040  | USB UART   | TDR2/RDR2 | UART 2 data register         |
| 0xA041  | USB UART   | RST2/SR2  | UART 2 reset/status register |
| 0xA042  | USB UART   | CMDR2     | UART 2 command register      |
| 0xA043  | USB UART   | CR2       | UART 2 control register      |

