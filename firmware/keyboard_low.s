; FellaPC low level keyboard scan
; A.K. 2024

.import			_keymap_main, _keymap_alt
.import			_keyboard_keys

.importzp		ptr1, ptr2, ptr3, tmp1, tmp2, tmp3, tmp4

.define			i tmp1
.define			j tmp2
.define			isFn ptr3
.define			change tmp3
.define			state tmp4
.define			main ptr1
.define			alt ptr2

.export 		_keyboard_scan

.proc			_keyboard_scan: near

.segment		"BSS"

rowLast:		.res 16, $00
row:			.res 16, $00

.segment		"CODE"

				; scan the keyboard
				LDY #15
@scan:			LDA $6C00, Y
				EOR #$FF
				AND #$1F
				STA row, Y
				DEY
				BPL @scan

				; prepare map pointers
				LDA #<_keymap_main
				STA main
				LDA #>_keymap_main
				STA main + 1
				LDA #<_keymap_alt
				STA alt
				LDA #>_keymap_alt
				STA alt + 1

				; is Fn pressed?
				LDA row + 9
				AND #$10
				STA isFn

				; row iterator
				LDA #0
				STA i

@rowLoop:		; change = row ^ rowLast
				LDY i
				LDA row, Y
				EOR rowLast, Y
				STA change
				LDA row, Y
				STA rowLast, Y

				; for each column
				LDA #0
				STA j

@colLoop:		; !change[j] continue
				LDA change
				AND #1
				BEQ @colLoopEnd

				; pressed or released?
				LDY i
				LDA row, Y
				AND #1
				STA state

@getCode:		; get key code (map[j][i])
				LDY j
				LDA #0
				CLC
@mul:			DEY
				BMI @mulend
				ADC #16
				BNE @mul

@mulend:		ADC i
				TAY

				; check if Fn is active, change map if so
				LDA isFn
				BEQ @noFn

				LDA (alt), Y
				JMP @gotCode

@noFn:			LDA (main), Y

				; is modifier?
@gotCode:		CMP #$E0
				BCC @notMod
				CMP #$FF
				BEQ @notMod

				; convert key to modifier
				AND #$0f
				TAX
				LDA #1
@modloop:		CPX #0
				BEQ @modloopend
				ASL
				DEX
				BNE @modloop

@modloopend:	; if key active
				LDY state
				BEQ @modDisable
				ORA _keyboard_keys

@modEnd:		STA _keyboard_keys
				JMP @colLoopEnd

@modDisable:	EOR #$FF
				AND _keyboard_keys
				JMP @modEnd

@notMod:		; if key active
				LDY state
				BNE @active

				LDA #0
				STA _keyboard_keys + 1
				JMP @colLoopEnd

@active:		LDY _keyboard_keys + 1
				BNE @overflow
				STA _keyboard_keys + 1

@colLoopEnd:	INC j
				LDX i
				LSR row, X
				LSR change
				BNE @colLoop

				INC i

				; i < 16?
				LDA i
				CMP #16
				BEQ @end

				JMP @rowLoop

@end:			LDA #0
				TAX
				RTS

@overflow:		; overflow - clear everything
				LDA #0
				LDY #15
				STA _keyboard_keys
				STA _keyboard_keys + 1

@ovflLoop:		STA rowLast, Y
				DEY
				BPL @ovflLoop

				LDA #$FF
				TAX
				RTS

.endproc
