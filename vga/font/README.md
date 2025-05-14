# Font ROM

Font taken from https://github.com/idispatch/raster-fonts/blob/master/font-8x8.c

VGA test can handle up to 4 fonts:
- 0: white on black, 256 chars (+ special chars),
- 1: black on white, 256 chars (+ special chars),
- 2: Only lower 128 chars, lower half white on black, upper black on white,
- 3: Only lower 128 chars, lower half black on white, upper white on black.

## gen.c

Generates binary font, ready to be programmed into an EPROM.

## rom.bin

Ready to be programmed binary font

