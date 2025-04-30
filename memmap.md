# Memory map

| Start  | End    | Description |
|--------|--------|-------------|
| 0x0000 | 0x5FFF | RAM         |
| 0x6000 | 0x7FFF | I/O         |
| 0x8000 | 0xFFFF | ROM         |

# I/O register map

| Address | Peripheral | Register  | Description                  |
|---------|------------|-----------|------------------------------|
| 0x6000  | VGA        | VDATA     | VGA data (128 registers)     |
| 0x6400  | VGA        | VRA       | VGA row address              |
| 0x6800  | VGA        | VSCRL     | VGA vertical scroll          |
| 0x6C00  | Keyboard   | COLx      | Keyboard columns (0-15)      |
| 0x7000  | User port  | ORB       | Port B output register       |
| 0x7001  | User port  | ORA       | Port A output register       |
| 0x7002  | User port  | DDRB      | Port B direction register    |
| 0x7003  | User port  | DDRA      | Port A direction register    |
| 0x7004  | User port  | T1(L/C)-L | Timer 1 low latch/counter    |
| 0x7005  | User port  | T1C-H     | Timer 1 high counter         |
| 0x7006  | User port  | T1L-L     | Timer 1 low latch            |
| 0x7007  | User port  | T1L-H     | Timer 1 high latch           |
| 0x7008  | User port  | T2(L/C)-L | Timer 2 low latch/counter    |
| 0x7009  | User port  | T2C-H     | Timer 2 high counter         |
| 0x700A  | User port  | SR        | Shift register               |
| 0x700B  | User port  | ACR       | Auxiliary control register   |
| 0x700C  | User port  | PCR       | Peripheral control register  |
| 0x700D  | User port  | IFR       | Interrupt flag register      |
| 0x700E  | User port  | IER       | Interrupt enable register    |
| 0x700F  | User port  | ORA       | Port A output register       |
| 0x7400  | USB UART   | TDR1/RDR1 | UART 1 data register         |
| 0x7401  | USB UART   | RST1/SR1  | UART 1 reset/status register |
| 0x7402  | USB UART   | CMDR1     | UART 1 command register      |
| 0x7403  | USB UART   | CR1       | UART 1 control register      |
| 0x7800  | RS232      | TDR2/RDR2 | UART 2 data register         |
| 0x7801  | RS232      | RST2/SR2  | UART 2 reset/status register |
| 0x7802  | RS232      | CMDR2     | UART 2 command register      |
| 0x7803  | RS232      | CR2       | UART 2 control register      |

