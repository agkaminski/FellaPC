; FellaPC DRAM refresh
; A.K. 2024

; Designed for TMS4416 DRAMs,
; iterate over A0-A7 row address.

.export			_dram_refresh

.proc			_dram_refresh: near

.segment		"ZEROPAGE"

@counter:		.res 1, $00

.segment		"CODE"

				LDY 0
@loop:			LDA (@counter), Y
				INY
				BNE @loop
				RTS
.endproc