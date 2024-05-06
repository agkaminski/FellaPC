; FellaPC ASM helper functions
; A.K. 2024

.importzp		sp, ptr1, ptr2, ptr3

.export			__ptrPrologue, __2ptrPrologue, __indPrologue

; f(*arg) -> ptr1 = arg
.proc			__ptrPrologue: near

				STA ptr1
				STX ptr1 + 1
				LDY #0
				LDX #0
				RTS

.endproc


; f(*arg1, *arg2) -> ptr1 = arg2, ptr2 = arg1
; restore stack
.proc			__2ptrPrologue: near

				JSR __ptrPrologue

				LDA (sp), Y
				STA ptr2
				INC sp
				LDA (sp), Y
				STA ptr2 + 1
				INC sp
				RTS

.endproc

; __2ptrPrologue + ptr3 = *ptr2
.proc			__indPrologue: near

				JSR __2ptrPrologue

				; ptr3 = *list
				LDA (ptr2), Y
				STA ptr3
				INY
				LDA (ptr2), Y
				STA ptr3 + 1
				DEY
				RTS

.endproc
