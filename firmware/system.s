; FellaPC system init
; A.K. 2024

.import		__BSS_RUN__, __BSS_SIZE__
.import		__DATA_SIZE__
.import		__STACKSIZE__, __RAM_SIZE__
.import		_vga_selectRom, _ualloc_init
.import		pushax

.export		_system_init

.segment	"CODE"

.proc		_system_init: near

			LDA #0
			JSR _vga_selectRom

			LDA #<(__BSS_RUN__ + __BSS_SIZE__)
			LDX #>(__BSS_RUN__ + __BSS_SIZE__)
			JSR pushax

			LDA #<(__RAM_SIZE__ - __BSS_SIZE__ - __DATA_SIZE__ - __STACKSIZE__)
			LDX #>(__RAM_SIZE__ - __BSS_SIZE__ - __DATA_SIZE__ - __STACKSIZE__)
			JMP _ualloc_init

.endproc
