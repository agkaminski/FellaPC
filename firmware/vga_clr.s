; FellaPC vga_scroll
; A.K. 2024

; Sadly it is not possible to do this during vsync,
; visual glitches will be observed.

.export			_vga_clr

.proc			_vga_clr: near

.segment		"CODE"

				LDY #59
				LDA #$20

@row_loop:		STY $8002
				LDX #79
@col_loop:		STX $8001
				STA $8000
				DEX
				BPL @col_loop
				DEY
				BPL @row_loop

				RTS
.endproc
