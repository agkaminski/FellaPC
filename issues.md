# Issues found in rev A

- Wrong value of R5 and R6, is 10 K, should be 1K5,
- rotate U33 for more optimal routing,
- missing reset capacitor (NE555 pin 6, 7 to the ground, ~220nF),
- U29 (6522) has to have at least 2 MHz rating to work. Cosider reducing CPU clk.
