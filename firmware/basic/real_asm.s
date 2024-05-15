; FellaPC real numbers
; A.K. 2024

.export			__real_bcdAdd
.export			__real_bcdSub
.export			_real_isZero
.export			_real_shiftLeft
.export			_real_shiftRight
.export			_real_cmp
.export			_real_normalize
.export			_real_copy
.export			_real_setZero
.export			_real_setOne

.importzp		sp, ptr1, ptr2, tmp1, tmp2

.import			__ptrPrologue, __2ptrPrologue
.import			pushax, _memcpy

.import			_rzero, _rone

.segment		"CODE"

.proc			__real_bcdAdd: near

				JSR __2ptrPrologue

				SED
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

				JSR __2ptrPrologue

				SED
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

				JSR __ptrPrologue

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

				JSR __ptrPrologue

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

				JSR __ptrPrologue

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

				JSR __2ptrPrologue

				LDY #4

@loop:			LDA (ptr2), Y
				CMP (ptr1), Y

				BCC @less
				BNE @more

				DEY
				BPL @loop

				LDA #0
				RTS
@less:			LDA #255
				RTS
@more:			LDA #1
				RTS

.endproc

.proc			_real_normalize: near

				JSR _real_isZero

				; pointer moved to ptr1 by _real_isZero

				CMP #0
				BNE @iszero

@loop:			; check if MSB is non-zero

				LDY #4
				LDA (ptr1), Y
				AND #$F0
				BNE @end

				; decrease exponent only if e >= INT8_MIN

				LDY #5
				LDA (ptr1), Y
				CMP #$F0
				BEQ @end
				TAX
				DEX
				TXA
				STA (ptr1), Y

				LDA ptr1
				LDX ptr1 + 1
				JSR _real_shiftLeft
				JMP @loop

@iszero:		; fix zero number (set exponent and sign to default)

				LDA #0
				LDY #5
				STA (ptr1), Y
				LDA #1
				INY
				STA (ptr1), Y

@end:			RTS

.endproc

.proc			_real_copy: near

				JSR pushax
				LDA #7
				LDX #0
				JMP _memcpy

.endproc

.proc			_real_setZero: near

				JSR pushax
				LDA #<_rzero
				LDX #>_rzero
				JMP _real_copy

.endproc

.proc			_real_setOne: near

				JSR pushax
				LDA #<_rone
				LDX #>_rone
				JMP _real_copy

.endproc
