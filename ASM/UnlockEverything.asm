// Dream Tag Tournament - Unlock Everything
// For GameCube's official AGB emulator which doesn't support saving.
// This also disables saving, so no scores will be recorded.

.gba

.open "DTT.gba","DTT_UNLOCK_ALL.gba",0x08000000 


.org 0x080048CA
dh 0x0000     ;removes cmp for clearing savedata
dh 0x0000     ;removes

.org 0x08FFFDEC

// usable space
.area 0xF0

.align 4

.func UnlockAll
	;don't use r3
	
	;Settings
	;skip for now, settings can be done with ROM writes
	
	;100% collection
	ldr r0,=0x02022050
	mov r1,#0x64
	strb r1,[r0,0xF]
	
	;completed all raiku cup 1
	mov r1,#0xFF
	strb r1,[r0,0xC]
	
	;completed all raiku cup 2
	mov r1,#0xF
	strb r1,[r0,0xD]
	
	;all minigames and sound test
	mov r1,#0x1F
	strb r1,[r0,0xE]
	
	;Next section
	ldr r0,=0x02022070
	
	;chr set 1
	mov r1,#0x40
	strb r1,[r0,0x0]
	
	;chr set 2
	mov r1,#0xFC
	strb r1,[r0,0x1]
	
	;chr set 3
	mov r1,#0x7
	strb r1,[r0,0x2]
	
	;Stages 5-8
	mov r1,#0x78
	strb r1,[r0,0x4]
	
	;Stages 9-14
	mov r1,#0x3F
	strb r1,[r0,0x5]
	
	;unko tintin hard, lower 4 bits is the position
	mov r1,#0x12
	strb r1,[r0,0x6]
	
	;punish momon hard
	mov r1,#0x12
	strb r1,[r0,0x7]
	
	;koral q hard
	mov r1,#0x12
	strb r1,[r0,0x8]
	
	;faudo trial hard
	mov r1,#0x12
	strb r1,[r0,0x9]
	
	;score for unkotintin, easy, short
	;mov r1,#0x35
	;strb r1,[r0,0xA]
	
	;back to normal
	ldr r0,=0x02022050
	
	;dream tag badge
	mov r1,#0x1F
	strb r1,[r0,0x1C]
	
	;unlock items
	mov r4,#0xF
	b ITEMS_GET
	
	
	;b HOLD_ON

;HOLD_ON:
;	b HOLD_ON

ITEMS_GET:
	add r4,r4,#1
	mov r1,#0xFF
	strb r1,[r0,r4]
	cmp r4,0x1B
	blt ITEMS_GET
	b finish

finish:
	;Original code
	mov r0,#0x0
	str r0,[r3,#0x4]
	ldr r0,=0x080049DC
	str r0,[r3,#0x0]
	
	ldr r2,=0x080049B6  ;exit back
	mov r15,r2          ;r15 is the pc, it's an alternate long branch

.pool
.endfunc
.endarea


// Hooking into the save initializer
.org 0x080049AE
	ldr r2,=0x08FFFDEC
	mov r15,r2

.pool

.close