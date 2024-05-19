; FellaPC I2C EEPROM driver
; A.K. 2024

; PB6 SDA
; PB7 SCL

.importzp		tmp1, tmp2, ptr1, ptr2, ptr3, sp

.define			ORB  $A010
.define			DDRB $A012
.define			SDA $40
.define			SCL $80
.define			nSDA $BF
.define			nSCL $7F
.define			DEVADDR $A0
.define			MEMSZ $8000

.define			dev tmp2
.define			addr ptr1
.define			len ptr2
.define			data ptr3

.segment		"CODE"

.proc			wait: near

				LDA #255
				SEC
@loop:			SBC #1
				BNE @loop
				RTS

.endproc

.proc			sda_on: near

				LDA ORB
				ORA #SDA
				STA ORB
				LDA DDRB
				AND #nSDA
				STA DDRB
				RTS

.endproc

.proc			sda_off: near

				LDA ORB
				AND #nSDA
				STA ORB
				LDA DDRB
				ORA #SDA
				STA DDRB
				RTS

.endproc

.proc			scl_on: near

				LDA ORB
				ORA #SCL
				STA ORB
				LDA DDRB
				AND #nSCL
				STA DDRB
				RTS

.endproc

.proc			scl_off: near

				LDA ORB
				AND #nSCL
				STA ORB
				LDA DDRB
				ORA #SCL
				STA DDRB
				RTS

.endproc

.proc			sda_read: near

				LDA ORB
				AND #SDA
				RTS

.endproc

.proc			scl_read: near

				LDA ORB
				AND #SCL
				RTS

.endproc

.proc			i2c_generateStart: near

				JSR scl_on
				JSR sda_on
				JSR sda_off
				JSR scl_off
				RTS

.endproc

.proc			i2c_generateStop: near

				JSR sda_off
				JSR scl_on
				JSR sda_on
				RTS

.endproc

.proc			i2c_sendByte: near

				STA tmp1
				LDY #7

@loop:			AND #$80
				BEQ @zero
				JSR sda_on
				JMP @next

@zero:			JSR sda_off
@next:			LDA tmp1
				ASL
				STA tmp1

				JSR scl_on
				JSR scl_off

				LDA tmp1
				DEY
				BPL @loop

				JSR sda_on
				JSR scl_on
				JSR scl_off
				RTS

.endproc

.proc			i2c_getByte: near

				PHA ; ack flag
				JSR sda_on

				LDA #0
				STA tmp1
				LDY #7
@loop:			JSR scl_on

				LDA tmp1
				ASL
				STA tmp1

				JSR sda_read
				BEQ @skip

				LDA #1
				ORA tmp1
				STA tmp1

@skip:			JSR scl_off
				DEY
				BPL @loop

				PLA
				BEQ @ackskip

				JSR sda_off

@ackskip:		JSR scl_on
				JSR scl_off

				LDA tmp1
				RTS

.endproc

.proc			i2c_address: near

				; dev and addr in zeropage regs

				JSR i2c_generateStart

				LDA dev
				JSR i2c_sendByte

				LDA #1
				AND dev
				BNE @end

				LDA addr
				JSR i2c_sendByte

				LDA addr + 1
				JSR i2c_sendByte

@end:			RTS

.endproc

.proc			i2c_prologue: near

				STA len
				STX len + 1

				LDY #0
				LDA (sp), Y
				STA data
				INC sp
				LDA (sp), Y
				STA data + 1
				INC sp
				LDA (sp), Y
				STA addr
				INC sp
				LDA (sp), Y
				STA addr + 1
				INC sp

				RTS

.endproc

.proc			i2c_nextByte: near

				SEC
				LDA #0
				ADC addr
				STA addr
				LDA #0
				ADC addr + 1
				STA addr + 1

				SEC
				LDA #0
				ADC data
				STA data
				LDA #0
				ADC data + 1
				STA data + 1

				CLC
				LDA #0
				SBC len
				STA len
				LDA #0
				SBC len + 1
				STA len + 1
				RTS

.endproc

.proc			_i2c_write: near

				JSR i2c_prologue

@loop:			LDA #DEVADDR
				STA dev
				JSR i2c_address

				LDY #0
				LDA (data), Y
				JSR i2c_sendByte

				JSR i2c_generateStop

				JSR i2c_nextByte

				LDA len
				ORA len + 1
				BNE @loop

				RTS

.endproc

.proc			_i2c_read: near

				JSR i2c_prologue

				LDA #DEVADDR
				STA dev
				JSR i2c_address

				LDA #DEVADDR | 1
				STA dev
				JSR i2c_address

@loop:			LDA #1
				JSR i2c_getByte
				LDY #0
				STA (data), Y

				JSR i2c_nextByte

				LDA len + 1
				BNE @loop
				LDA len
				CMP #1
				BNE @loop

				LDA #0
				JSR i2c_getByte
				LDY #0
				STA (data), Y

				RTS

.endproc


