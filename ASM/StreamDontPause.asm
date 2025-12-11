// DTT Stream Playing Enhancement
// Prevents pausing streamed BGM during gameplay because it can't be resumed
// but keeps pausing for regular sequenced music. Takes up 0x30 bytes.

.gba

.open "dtt.gba","DTT_enhancement.gba",0x08000000 


//SaveFlash EQU 0x08001FCD
;.org 0x08000510
;dh 0xE00A     ;branches to the bl that writes the savefile


// These two instructions become redundant, so nop them off
.org 0x08004A3C
dw 0x00000000


.org 0x08FFFE40

.area 0x1C0

.align 4

.func PauseMusicConditional
	;Replacement of original functions
	ldr r1,=0x02021F00
	ldr r0,=0x02022048
	
	;Original functions
	ldrh r0,[r0,#0]
	lsl r0, r0, #1
	add r0,r0,r1
	ldrh r0,[r0,#0]
	
	;new code--if it's 09 addr it's a stream
	ldr r2,=0x020019B3
	ldrb r2,[r2]
	cmp r2,#08
	bgt isStream
	
	;it's ok to pause
	ldr r1,=0x08004A48
	mov r15,r1

isStream:
	ldr r1,=0x08004A4C ;it's NOT ok to pause
	mov r15,r1         ;r15 is the pc, it's an alternate long branch

.pool
.endfunc
.endarea


// Hooking into the pause game code
.org 0x08004A40
	ldr r0,=0x08FFFE40
	mov r15,r0

.pool

.close