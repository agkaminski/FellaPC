; FellaPC low level BCD add/sub
; A.K. 2024

.export			__real_bcdAdd
.export			__real_bcdSub

.importzp		sp, ptr1, ptr2

.segment		"CODE"

.proc			__real_bcdPrologue: near

				STA ptr1
				STX ptr1 + 1
				LDX #0
				LDY #0
				LDA (sp), Y
				STA ptr2
				INY
				LDA (sp), Y
				STA ptr2 + 1
				DEY
				INC sp
				INC sp
				SED
				RTS

.endproc

.proc			__real_bcdAdd: near

				JSR __real_bcdPrologue

				CLC
				PHP

@loop:			LDA (ptr2), Y
				PLP
				ADC (ptr1), Y
				PHP
				STA (ptr2), Y
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

				JSR __real_bcdPrologue

				SEC
				PHP

@loop:			LDA (ptr2), Y
				PLP
				SBC (ptr1), Y
				PHP
				STA (ptr2), Y
				INY
				CPY #5
				BNE @loop
				PLP

				CLD
				RTS

.endproc
