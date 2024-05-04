; FellaPC real numbers
; A.K. 2024

.export			__real_bcdAdd
.export			__real_bcdSub
.export			_real_isZero
.export			_real_shiftLeft
.export			_real_shiftRight
.export			_real_cmp

.importzp		sp, ptr1, ptr2, tmp1, tmp2

.segment		"CODE"

.proc			_real_ptrPrologue: near

				STA ptr1
				STX ptr1 + 1
				LDY #0
				LDX #0
				RTS

.endproc

.proc			_real_2ptrPrologue: near

				JSR _real_ptrPrologue
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

				JSR _real_2ptrPrologue

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

				JSR _real_2ptrPrologue

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

.proc			_real_isZero: near

				JSR _real_ptrPrologue

@loop:			LDA (ptr1), Y
				BNE @nope
				INY
				CPY #5
				BNE @loop
				LDA #1
				RTS

@nope:			LDA #0
				RTS

.endproc

.proc			_real_shiftLeft: near

				JSR _real_ptrPrologue

@loop:			LDY #0
				PHP
				CLC

@mloop:			PLP
				LDA (ptr1), Y
				ROL
				STA (ptr1), Y
				PHP
				INY
				CPY #5
				BNE @mloop

				PLP
				INX
				CPX #4
				BNE @loop

				RTS

.endproc

.proc			_real_shiftRight: near

				JSR _real_ptrPrologue

@loop:			LDY #4
				CLC

@mloop:			LDA (ptr1), Y
				ROR
				STA (ptr1), Y
				DEY
				BPL @mloop

				INX
				CPX #4
				BNE @loop

				RTS

.endproc

.proc			_real_cmp: near

				JSR _real_2ptrPrologue

				LDY #4

@loop:			LDA (ptr2), Y
				CMP (ptr1), Y

				BCS @more
				BNE @less

				DEY
				BPL @loop

				LDA #0
				RTS
@less:			LDA #255
				RTS
@more:			LDA #1
				RTS

.endproc
