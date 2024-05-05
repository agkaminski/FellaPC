; FellaPC low level keyboard scan
; A.K. 2024

; Has to be in asm! cc65 generates (zp), Y addressing
; mode, which reads some random memory during execution.
; It's ok for memory access, but disastrous for I/O.
; It caused VGA registers to be modified (as some
; of them do not handle reads and access is always
; treated as write).

.export			_keyboard_scanLow
.importzp		ptr1, ptr2

.segment		"CODE"

.proc			_keyboard_scanLow: near

				STA ptr1
				STX ptr1 + 1
				LDY #15

@loop:			LDA $9000, Y
				EOR #$FF
				STA (ptr1), Y
				DEY
				BPL @loop

				RTS

.endproc
