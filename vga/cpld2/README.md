# CPLD - VGA timing generator and position counter

Target: XC9536, any speed, **not** XL version

## Pinout

| Pin | Type     | Name      |
|-----|----------|-----------|
| 1   | I/O      | vsync     |
| 2   | I/O      | hsync     |
| 3   | I/O      | blank     |
| 4   | I/O      | shload_n  |
| 5   | I/O/GCK1 | pclk      |
| 6   | I/O/GCK2 | TIE       |
| 7   | I/O/GCK3 | TIE       |
| 8   | I/O      | col<9>    |
| 9   | I/O      | col<8>    |
| 10  | GND      | GND       |
| 11  | I/O      | col<7>    |
| 12  | I/O      | col<6>    |
| 13  | I/O      | col<5>    |
| 14  | I/O      | col<4>    |
| 15  | TDI      | TDI       |
| 16  | TMS      | TMS       |
| 17  | TCK      | TCK       |
| 18  | I/O      | col<3>    |
| 19  | I/O      | vblank_n  |
| 20  | I/O      | scroll<0> |
| 21  | VCCINT   | VCC       |
| 22  | I/O      | scroll<1> |
| 23  | GND      | GND       |
| 24  | I/O      | scroll<4> |
| 25  | I/O      | row<8>    |
| 26  | I/O      | row<7>    |
| 27  | I/O      | row<6>    |
| 28  | I/O      | row<5>    |
| 29  | I/O      | row<4>    |
| 30  | TDO      | TDO       |
| 31  | GND      | GND       |
| 32  | VCCIO    | VCC       |
| 33  | I/O      | row<3>    |
| 34  | I/O      | row<2>    |
| 35  | I/O      | row<1>    |
| 36  | I/O      | row<0>    |
| 37  | I/O      | oe_n      |
| 38  | I/O      | scroll<5> |
| 39  | I/O/GSR  | rst_n     |
| 40  | I/O/GTS2 | TIE       |
| 41  | VCCINT   | VCC       |
| 42  | I/O/GTS1 | TIE       |
| 43  | I/O      | scroll<2> |
| 44  | I/O      | scroll<3> |

## vga.v

Implementation

## vga.ucf

Pinout constraints (ISE 14.7)

## vga.jed

Bitstream ready to upload

## tb.v

Sorry excuse for a testbench
