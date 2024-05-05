; FellaPC VGA asm
; A.K. 2024

.export			_vga_resetCursor
.export			_vga_handleCursor
.export			_vga_newLine
.export			_vga_vsync
.export			_vga_clear
.export			_vga_scroll
.export			_vga_putLine
.export			_vga_set
.export			_vga_get
.export			_vga_incCol
.export			_vga_putc
.export			_vga_puts
.export			_vga_selectRom

.exportzp		_g_cursor_col
.exportzp		_g_cursor_row

.importzp		tmp1, tmp2, ptr1
.importzp		_g_vsync

.define			VDATA $8000
.define			VCOL $8001
.define			VROW $8002

.define			COLS 80
.define			ROWS 60
.define			CURSOR 219

.define			ORB $A010
.define			DDRB $A012

.define			CR $0D
.define			LF $0A
.define			HT $09

.segment		"ZEROPAGE"

_g_cursor_col:	.res 1, $0
_g_cursor_row:	.res 1, $0

cursor_counter:	.res 1, $0
cursor_prev:	.res 1, $0
cursor_state:	.res 1, $0

.segment		"CODE"

.proc			_vga_vsync: near

				LDA #0
				STA _g_vsync
@loop:			LDA _g_vsync
				BEQ	@loop
				RTS

.endproc

.proc			_vga_setRowCol: near

				JSR _vga_vsync
				LDA _g_cursor_row
				STA VROW
				LDA	_g_cursor_col
				STA VCOL
				RTS

.endproc

.proc			_vga_handleCursor: near

				INC cursor_counter
				LDA #32
				CMP cursor_counter
				BNE @restore

				LDA #0
				STA cursor_state
				JSR _vga_get
				STA cursor_prev
				LDX #CURSOR
				STX cursor_state
				JSR _vga_setRowCol
				STX VDATA
				RTS

@restore:		ASL A
				CMP cursor_counter
				BEQ _vga_resetCursor
				RTS

.endproc

.proc			_vga_resetCursor: near

				LDA cursor_state
				BEQ @end

				LDA cursor_prev
				JSR _vga_set

@end:			LDA #0
				STA cursor_counter
				STA cursor_state
				RTS

.endproc

; Sadly it is not possible to do this during vsync,
; visual glitches will be observed.

.proc			_vga_clear: near

				JSR _vga_resetCursor

				LDA #0
				STA _g_cursor_col
				STA _g_cursor_row

				LDY #ROWS - 1
				LDA #$20

@row_loop:		STY VROW
				LDX #COLS - 1
@col_loop:		STX VCOL
				STA VDATA
				DEX
				BPL @col_loop
				DEY
				BPL @row_loop

				RTS
.endproc

; Sadly it is not possible to do this during vsync,
; visual glitches will be observed.
; Consider using HW scrolling in future HW.

.proc			_vga_scroll: near

				; row
				LDY #1
				STY VROW

				; col
				LDX #0

@row_loop0:		STX VCOL
				LDA VDATA
				PHA
				INX
				CPX #COLS
				BNE @row_loop0

				DEX
				DEY
				STY VROW

@row_loop1:		PLA
				STA VDATA
				DEX
				STX VCOL
				CPX #255
				BNE @row_loop1

				INY
				INY
				INX
				STY VROW
				CPY #ROWS
				BNE @row_loop0

				; clear last row
				LDA #$20
				DEY
				STY VROW
@clr_loop:		STX VCOL
				STA VDATA
				INX
				CPX #COLS
				BNE @clr_loop

				RTS
.endproc

.proc			_vga_newLine: near

				JSR _vga_resetCursor
				LDA #0
				STA _g_cursor_col
				LDA #ROWS - 1
				CMP _g_cursor_row
				BEQ _vga_scroll

@addrow:		INC _g_cursor_row
				RTS

.endproc

.proc			_vga_putc: near

				TAY
				JSR _vga_resetCursor

				CPY #0
				BEQ @end

				CPY #LF
				BEQ _vga_newLine

				LDA #3
				AND _g_cursor_col
				STA tmp1
				LDA #4
				SEC
				SBC tmp1

				CPY #HT
				BEQ _vga_incCol

				TYA
				JSR _vga_set

				LDA #1
				BNE _vga_incCol

@end:			RTS

.endproc

.proc			_vga_incCol: near

				CLC
				ADC _g_cursor_col
				STA _g_cursor_col
				LDA #COLS - 1
				CMP _g_cursor_col
				BCC _vga_newLine
				RTS

.endproc

;We strongly assume that line can fit within current column

.proc			_vga_putLine: near

				STA ptr1
				STX ptr1 + 1

				JSR _vga_setRowCol
				LDX _g_cursor_col

				LDY #0
				LDX _g_cursor_col
@loop:			LDA (ptr1), Y
				BEQ @end
				STA VDATA
				INY
				INX
				STX VCOL
				BNE @loop ; always non-zero, save one byte over JMP
@end:			TYA
				LDX #0
				RTS

.endproc

.proc			_vga_set: near

				TAX
				JSR _vga_setRowCol
				STX VDATA
				STX cursor_prev
				RTS

.endproc

.proc			_vga_get: near

				LDX #0
				LDA cursor_state
				BEQ @hwaccess
				LDA cursor_prev
				RTS

@hwaccess:		JSR _vga_setRowCol
				LDA VDATA
				RTS

.endproc

.proc			_vga_puts: near

				STA ptr1
				STX ptr1 + 1

				JSR _vga_resetCursor
				JSR _vga_setRowCol

				LDY #0
@loop:			LDA (ptr1), Y
				BEQ @end

				INY

				CMP #LF
				BEQ @putc
				CMP #CR
				BEQ @putc
				CMP #HT
				BEQ @putc

				LDX _g_cursor_col
				CPX #COLS - 1
				BEQ @putc

				; Row should be already set
				STX VCOL
				STA  VDATA
				INC _g_cursor_col
				BNE @loop ; Always true

@putc:			STY tmp2
				JSR _vga_putc
				JSR _vga_setRowCol
				LDY tmp2
				JMP @loop

@end:			RTS

.endproc

.proc			_vga_selectRom: near

				ASL
				ASL
				ASL
				ASL
				AND #$30
				PHA
				LDA DDRB
				ORA #$30
				STA DDRB
				LDA ORB
				AND #$CF
				STA tmp1
				PLA
				ORA tmp1
				STA ORB
				RTS

.endproc

