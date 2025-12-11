// Dream Tag Tournament - Dream Tag Badge unlocking
// Hooks to the sound test code to allow the user to play a certain
// combination of voices to unlock the badge in the options mode.
// The player must have 100 percent in the Zatch Collection first.

// The sounds are: 002, 147, 036, 009 (DTT/Yellowtail!, RajaMigsen!, Sore!/TakeOff, SugoiNoDa!/Perfect!)
// 'Raja Migsen!' and 'Sore!' are unused sounds.


.gba

.open "DTT.gba","DTT_BADGE.gba",0x08000000 


.org 0x08FFFAF0

// usable space
.area 0xBC

.align 4

.func DreamTagBadge
	;Original code
	str r0,[r7,#0x08]
	mov r0,#0xFF
	strb r0,[r7,#0x1B]
	add r0,r6,#0x0
	
	;r0 contains the sound id
	
	;Zatch Collection percentage byte is at 0202205F
	;Badge is at the 4th bit of the byte at 0202206C (0x0F -> 0x1F)
	;03001BB0 = good spot for password counter
	;r3 might be usable
	
	ldr r1,=0x0202205F  ;percentage
	ldrb r6,[r1]        ;read it
	
	cmp r6,#0x64        ;ignore if collection percentage is not there yet
	blt finish
	
	;badge-have check
	;emptyyyyyy
	
	ldr r1,=0x03001BB0  ;space
	ldrb r6,[r1]        ;read it
	
	;check if the password is done, do this on last pass
	;cmp r6,4
	;beq BADGE_GET
	
	ldr r3,=0x012E      ;002 VO
	cmp r0,r3
	beq PASS_1
	
	ldr r3,=0x0211      ;147 VO
	cmp r0,r3
	beq PASS_2
	
	ldr r3,=0x0150      ;036 VO
	cmp r0,r3
	beq PASS_3
	
	ldr r3,=0x0135      ;009 VO
	cmp r0,r3
	beq PASS_4
	
	b cleanup

PASS_1:
	add r6,#1           ;increase it
	cmp r6,1
	bne cleanup
	strb r6,[r1]
	b finish

PASS_2:
	add r6,#1           ;increase it
	cmp r6,2
	bne cleanup
	strb r6,[r1]
	b finish

PASS_3:
	add r6,#1           ;increase it
	cmp r6,3
	bne cleanup
	strb r6,[r1]
	b finish

PASS_4:
	add r6,#1           ;increase it
	cmp r6,4
	bne cleanup
	mov r6,#0
	strb r6,[r1]
	
	mov r0,#0x67        ;play cool sound effect as confirmation
	
	b BADGE_GET

BADGE_GET:
	ldr r3,=0x0202206C  ;DT badge is stored here
	;ldr r3,=0x03001BB4
	
	;Method 1, write the value alongside last chunk of items (2 bytes)
	;mov r6,#0x1F
	
	;Method 2, only set the bit that enables the badge (4 bytes)
	ldrb r6,[r3]
	mov r2,#0x10        ;OR params
	orr r6,r2           ;set bit 4 to unlock badge
	
	strb r6,[r3]
	
	b cleanup           ;returns and resets the used memory

cleanup:
	mov r6,#0
	strb r6,[r1]
	b finish

finish:
	;testing by crashing it
	;b finish
	
	ldr r1,=0x08015720  ;exit back
	mov r15,r1          ;r15 is the pc, it's an alternate long branch

.pool
.endfunc
.endarea


// Hooking into the sound test mode
.org 0x08015718
	ldr r1,=0x08FFFAF0
	mov r15,r1

.pool

.close