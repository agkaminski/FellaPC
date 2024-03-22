; FellaPC interrupts
; Some code based on the guide: https://cc65.github.io/doc/customizing.html
; A.K. 2024

; ---------------------------------------------------------------------------
; interrupt.s
; ---------------------------------------------------------------------------

.segment "BSS"

; Vars here

.segment  "CODE"

; ---------------------------------------------------------------------------
; Non-maskable interrupt (NMI) service routine

.proc _nmi_int: near

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
                RTI
.endproc

; Defines the interrupt vector table.

.segment  "VECTORS"

.import    _init

.addr      _nmi_int    ; NMI vector
.addr      _init       ; Reset vector
.addr      _irq_int    ; IRQ/BRK vector
