; FellaPC vga_scroll
; A.K. 2024

; Sadly it is not possible to do this during vsync,
; visual glitches will be observed.
; Consider using HW scrolling in future HW.

.export			_vga_scroll

.proc			_vga_scroll: near

.segment		"CODE"

				; row
				LDY #1
				STY $8002

				; col
				LDX #0

@row_loop0:		STX $8001
				LDA $8000
				PHA
				INX
				CPX #80
				BNE @row_loop0

				DEX
				DEY
				STY $8002

@row_loop1:		PLA
				STA $8000
				DEX
				STX $8001
				CPX #255
				BNE @row_loop1

				INY
				INY
				INX
				STY $8002
				CPY #60
				BNE @row_loop0

				; clear last row
				LDA #$20
				DEY
				STY $8002
@clr_loop:		STX $8001
				STA $8000
				INX
				CPX #80
				BNE @clr_loop

				RTS
.endproc
