# Issues found in rev A

- Wrong value of R5 and R6, is 10 K, should be 1K5,
- rotate U33 for more optimal routing,
- missing reset capacitor (NE555 pin 6, 7 to the ground, ~220nF),
- U29 (6522) has to have at least 2 MHz rating to work. Cosider reducing CPU clk,
- fatal misuse of 6502 CPU bus - chip selects should not be gated by PH2.
It only affects working of U29 (VIA). Fix: Disconnect pin 4 of U3 (/E0 of 74HC138)
from pin 2 of U6 (/PH2) and connect pin 4 of U3 to ground (pin 3 of U3).

