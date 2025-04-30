; FellaPC interrupts
; Some code based on the guide: https://cc65.github.io/doc/customizing.html
; A.K. 2024

; ---------------------------------------------------------------------------
; interrupt.s
; ---------------------------------------------------------------------------

; ---------------------------------------------------------------------------
; Non-maskable interrupt (NMI) service routine

.segment		"ZEROPAGE"

_g_vsync:		.res 1
jiffies:		.res 2

.exportzp		_g_vsync
.exportzp		jiffies

.segment		"CODE"

.proc _nmi_int: near

				PHA
				TXA
				PHA
				TYA
				PHA
				CLD

				; Set vsync flag
				LDA #$FF
				STA _g_vsync

				; Check for BRK
				TSX
				LDA $0104, X
				AND #$10
				BNE _break

				; Increment jiffies
				CLC
				LDA #1
				ADC jiffies
				STA jiffies
				LDA #0
				ADC jiffies + 1
				STA jiffies + 1

				PLA
				TAY
				PLA
				TAX
				PLA
				RTI
.endproc

; ---------------------------------------------------------------------------
; Maskable interrupt (IRQ) service routine

.proc _irq_int: near

				PHA
				TXA
				PHA
				TYA
				PHA
				CLD

				; Check for BRK
				TSX
				LDA $0104, X
				AND #$10
				BNE _break

				; TODO

				PLA
				TAY
				PLA
				TAX
				PLA
				RTI
.endproc

; ---------------------------------------------------------------------------
; BRK

.proc _break: near
				; TODO

				PLA
				TAY
				PLA
				TAX
				PLA

				RTI
.endproc

; Defines the interrupt vector table.

.segment	"VECTORS"

.import		_init

.addr		_nmi_int    ; NMI vector
.addr		_init       ; Reset vector
.addr		_irq_int    ; IRQ/BRK vector
