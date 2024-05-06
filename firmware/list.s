; FellaPC real numbers
; A.K. 2024

.importzp		sp, ptr1, ptr2, ptr3
.import			__2ptrPrologue, __indPrologue

.export			__list_push, __list_append, __list_pop

.segment		"CODE"

.proc			__list_push: near

				JSR __indPrologue
				; fall through

.endproc

.proc			__list_pushInternal: near

				; elem->next = *list
				LDA ptr3
				STA (ptr1), Y
				INY
				LDA ptr3 + 1
				STA (ptr1), Y

				; elem->prev = NULL
				LDA #0
				INY
				STA (ptr1), Y
				INY
				STA (ptr1), Y

				; if *list != NULL
				LDA ptr3
				ORA ptr3 + 1
				BEQ @null

				; *list->prev = elem
				DEY
				LDA ptr1
				STA (ptr3), Y
				INY
				LDA ptr1 + 1
				STA (ptr3), Y

@null:			; *list = elem
				LDY #0
				LDA ptr1
				STA (ptr2), Y
				INY
				LDA ptr1 + 1
				STA (ptr2), Y

				RTS

.endproc

.proc			__list_append: near

				JSR __indPrologue

				; if *list == NULL
				LDA ptr3
				BNE @loop
				LDA ptr3 + 1
				BNE @loop

				JMP __list_pushInternal

				; head = head->next
@next:			STA ptr3 + 1
				STX ptr3
				DEY

				; while (head->next != NULL)
@loop:			LDA (ptr3), Y
				TAX
				INY
				LDA (ptr3), Y
				BNE @next
				CPX #0
				BNE @next

				; head->next = t
				DEY
				LDA ptr1
				STA (ptr3), Y
				INY
				LDA ptr1 + 1
				STA (ptr3), Y

				; t->next = NULL
				DEY
				LDA #0
				STA (ptr1), Y
				INY
				STA (ptr1), Y
				INY

				; t->prev = head
				LDA ptr3
				STA (ptr1), Y
				INY
				LDA ptr3 + 1
				STA (ptr1), Y

				RTS

.endproc

.proc			__list_pop: near

				JSR __indPrologue

				; if (list != NULL)
				LDA ptr2
				ORA ptr2 + 1
				BEQ @l0

				; if (elem == *list)
				LDA ptr1
				CMP ptr3
				BNE @l0
				LDA ptr1 + 1
				CMP ptr3 + 1
				BNE @l0

				; *list is not needed anymore
				; ptr3 is free to use

				; *list = elem->next
				LDA (ptr1), Y
				STA (ptr2), Y
				INY
				LDA (ptr1), Y
				STA (ptr2), Y
				DEY

@l0:			; if (elem->next != NULL)
				LDA (ptr1), Y
				TAX
				INY
				LDA (ptr1), Y
				BNE @l1
				CPX #0
				BEQ @l2

@l1:			; elem->next->prev = elem->prev
				STA ptr3 + 1
				STX ptr3
				LDY #2
				LDA (ptr1), Y
				STA (ptr3), Y
				INY
				LDA (ptr1), Y
				STA (ptr3), Y

@l2:			; if (elem->prev != NULL)
				LDY #2
				LDA (ptr1), Y
				TAX
				INY
				LDA (ptr1), Y
				BNE @l3
				CPX #0
				BEQ @l4

@l3:			; elem->prev->next = elem->next
				STA ptr3 + 1
				STX ptr3
				LDY #0
				LDA (ptr1), Y
				STA (ptr3), Y
				INY
				LDA (ptr1), Y
				STA (ptr3), Y

@l4:			; elem->next = NULL, elem->prev = NULL
				LDY #3
				LDA #0
@l5:			STA (ptr1), Y
				DEY
				BPL @l5

				RTS

.endproc

