; FellaPC low level BCD add/sub
; A.K. 2024

.export			__real_bcdAdd
.export			__real_bcdSub

.import			__real_acc

.proc			__real_bcdAdd: near

.segment		"BSS"

@ptr:			.res 2

.segment		"CODE"

				STA @ptr
				STX @ptr + 1
				LDY #0
				CLC
				SED

@loop:			LDA __real_acc, Y
				ADC @ptr, Y
				STA __real_acc, Y
				INY
				CPY #5
				BNE @loop

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
				STA @ptr
				STX @ptr + 1
				LDY #0
				SEC
				SED

@loop:			LDA __real_acc, Y
				SBC @ptr, Y
				STA __real_acc, Y
				INY
				CPY #5
				BNE @loop

				CLD
				RTS

.endproc
