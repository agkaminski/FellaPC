; FellaPC low level BCD add/sub
; A.K. 2024

.export			__real_bcdAdd
.export			__real_bcdSub

.importzp		sp, ptr1

.proc			__real_bcdAdd: near

.segment		"CODE"

				STA ptr1
				STX ptr1 + 1
				LDX #0
				LDY #0
				CLC
				SED
				PHP

@loop:			LDA ptr1, Y
				PLP
				ADC (sp), Y
				PHP
				STA ptr1, Y
				INY
				CPY #5
				BNE @loop
				PLP

				LDA #0
				BCC @ncarry
				LDA #1

@ncarry:		CLD
				INC sp
				INC sp
				RTS

.endproc

.proc			__real_bcdSub: near

.segment		"BSS"

@ptr:			.res 2

.segment		"CODE"

				STA ptr1
				STX ptr1 + 1
				LDX #0
				LDY #0
				SEC
				SED
				PHP

@loop:			LDA ptr1, Y
				PLP
				SBC (sp), Y
				PHP
				STA ptr1, Y
				INY
				CPY #5
				BNE @loop
				PLP

				CLD
				INC sp
				INC sp
				RTS

.endproc
