; FellaPC low level BCD add/sub
; A.K. 2024

.export			__real_bcdAdd
.export			__real_bcdSub

.import			__real_acc
.importzp		sreg

.proc			__real_bcdAdd: near

.segment		"CODE"

				STA sreg
				STX sreg + 1
				LDX #0
				LDY #0
				CLC
				SED
				PHP

@loop:			LDA __real_acc, Y
				PLP
				ADC (sreg), Y
				PHP
				STA __real_acc, Y
				INY
				CPY #5
				BNE @loop
				PLP

				LDA #0
				BCC @ncarry
				LDA #1
@ncarry:		CLD
				RTS

.endproc

.proc			__real_bcdSub: near

.segment		"BSS"

@ptr:			.res 2

.segment		"CODE"

				STA sreg
				STX sreg + 1
				LDX #0
				LDY #0
				SEC
				SED
				PHP

@loop:			LDA __real_acc, Y
				PLP
				SBC (sreg), Y
				PHP
				STA __real_acc, Y
				INY
				CPY #5
				BNE @loop
				PLP

				CLD
				RTS

.endproc
