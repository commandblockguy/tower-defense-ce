.assume adl = 1
segment data
.def insertBoolArray
.def removeBoolArray


;void insertBoolArray(bool value, char* array, uint8_t index, uint8_t offset, uint8_t size);
insertBoolArray:
	; Preserve ix
	push	ix
	; Load the stack pointer into ix

	pop	ix	
	ret
; Arguments
; Stack is backwards
; There's a 3-byte return vector on top
.retVec	equ 0
.size	equ	.retVec+3
.offset	equ .size+1
.index	equ	.offset+1
.array	equ	.index+1
.value	equ	.array+3

;void removeBoolArray(char* array, uint8_t index, uint8_t offset, uint8_t size);
removeBoolArray:
	push	ix
	pop	ix
	ret