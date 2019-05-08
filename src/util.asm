.assume adl = 1
segment data
.def _shiftBitsRight
.def _shiftBitsLeft

openDebugger:
	scf
	sbc	hl,hl
	ld	(hl),2
	ret

;                             6              9
;void shiftBitsRight(uint8_t* array, uint8_t size)
_shiftBitsRight:
	; Preserve ix
	push	ix
	; Load the stack pointer into ix
	ld	ix,0
	add	ix,sp
	; Load array into hl
	ld	hl,(ix+6)
	; Load size into b
	ld	b,(ix+9)
_shiftBitsRight.loop
	; Shift everything right by one
	rr	(hl)
	; Move to the next byte
	inc	hl
	djnz	_shiftBitsRight.loop
	pop	ix	
	ret

;                            6              9
;void shiftBitsLeft(uint8_t* array, uint8_t size)
_shiftBitsLeft:
	; Preserve ix
	push	ix
	; Load the stack pointer into ix
	ld	ix,0
	add	ix,sp
	; Load size into b
	ld	b,(ix+9)
	; Load array+size-1 into hl
	ld	hl,(ix+6)
	ld	de,0
	ld	e,b
	add	hl,de
	dec	hl
_shiftBitsLeft.loop
	; Shift everything left by one
	rl	(hl)
	; Move to the next byte
	dec	hl
	djnz	_shiftBitsLeft.loop
	pop	ix	
	ret
