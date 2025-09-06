// DTT Character Intro Selector
// When the game plays the introduction voice (Zatch: ikuzo kiyomaro/Zeno: tokoton made butsuku)
// it does so for both P1 and P2, since all voices are on the same group, they override each other
// depending on the timing.

// So, for example: Riya's voice will always play no matter the combination
// because it plays very late in the intro (to the point where it gets cut off.)
// This makes it frustrating as some voices will hardly ever play or even get cut off
// by the other character's voice.

// To solve this I want to offer two options:
// 1. Mimic EA2 by only playing VO for P2.
// 2. Randomly choose the player.

// Since there is no easy way to read the current character in P2
// and doing so would require building a list of intro ids
// I decided to do method 2, which will use gameplay as a seed
// for randomizing which character's intro will play.
// I use the "Fight!" vo call to determine if the sound should skip at all.
// Then I use 3 different flag clear codes to reset the flag after a new match starts.

// This is working, but characters like Ponygon and Karudio
// who have double vo calls will conflict a bit.

.gba

.open "dtt.gba","DTT_enhancement.gba",0x08000000 


//SaveFlash EQU 0x08001FCD
;.org 0x08000510
;dh 0xE00A     ;branches to the bl that writes the savefile


// These two instructions become redundant, so nop them off
;.org 0x08004A3C
;dw 0x00000000


.org 0x08FFFE80

.area 0x1C0

.align 4

.func SelectVoiceIntro
	;Replacement of original functions
	push {lr}
	ldrh r2,[r1,#0xe]
	add r2,#0x2
	strh r2,[r1,#0xe]
	
	;ldrh r0,[r0,#0x4] ;keeping original
	;dh 0xF7F8 ;this one is just the first half of the bl
	
	;Check for Fight vo flag
	ldr r2,=0x020019A4  ;This spot will serve as our flag
	ldrh r1,[r2]        ;Read the current value into r1
	cmp r1,1
	beq playSound
	
	;Check for a spot in memory that always starts as 0
	ldr r2,=0x020019A0  ;This spot will serve as our flag
	ldrh r1,[r2]        ;Read the current value into r1
	
	;If 0, write 1
	cmp r1,0
	beq increase
	cmp r1,1
	beq decrease

increase:
	mov r1,#0x1
	b finalCompare

decrease:
	mov r1,#0x0
	b finalCompare

finalCompare:
	;store result
	str r1,[r2]
	
	;Final compare
	cmp r1,0
	beq playSound
	b skipSound
	
	
	;Load this addr into r3 and read val
;	ldr r3,=0x020019A0
;	ldr r4,[r3]
	
	;Invert r3 value
	;xor r4,#0x1
	
	;Skip selector code
;	beq skipSound
;	b playSound


skipSound:
	ldr r3,=0x08083AC2
	mov r15,r3

playSound:
	ldr r3,=0x08083ABC
	mov r15,r3

.pool
.endfunc
.endarea

.org 0x08083AB4
	ldr r3,=0x08FFFE80
	mov r15,r3

.pool


.org 0x08FFFEF0
.align 4

.func FlagFightVO
	;original
	strh r5,[r3,#0x0]
	mov r0,#0x20
	strh r0,[r2,#0x0]
	mov r0,#0xA0
	
	;write flag to mem
	ldr r1,=0x020019A4  ;This spot will serve as our flag
	;ldrh r1,[r2]        ;Read the current value into r1
	mov r2,#0x1
	str r2,[r1]
	
	;return
	ldr r1,=0x0801096c
	mov r15,r1

.pool
.endfunc

.org 0x08010964
	ldr r1,=0x08FFFEF0
	mov r15,r1

.pool


.org 0x08FFFF10
.align 4

.func ClearFlagNewMatch
	;original
	ldr r0,=0x02021F68
	strh r5,[r0,#0x0]
	ldr r0,=0x02021EA8
	strb r4,[r0,#0x0]
	
	;clear the fight! flag
	ldr r0,=0x020019A4
	mov r4,0
	str r4,[r0]
	
	;return
	ldr r0,=0x0800593C
	mov r15,r0

.pool
.endfunc

.org 0x08005934
	ldr r0,=0x08FFFF10
	mov r15,r0

.pool


.org 0x08FFFF40
.align 4

.func ClearFlagContinueYes
	;original
	;ldr r0,=0x08005984
	
	strh r5,[r0,#0x0]
	ldr r0,=0x02021EA8
	strb r4,[r0,#0x0]
	ldr r0,=0x02021F78
	
	;clear the fight! flag
	ldr r3,=0x020019A4
	mov r4,0
	str r4,[r3]
	
	;return
	ldr r3,=0x08007C58
	mov r15,r3

.pool
.endfunc

.org 0x08007C50
	ldr r3,=0x08FFFF40
	mov r15,r3

.pool


.org 0x08FFFF70
.align 4

.func ClearFlagDemoPlay
	;original
	strh r1,[r0,0x0]
	ldr r0,=0x020220D4
	strh r2,[r0,#0x0]
	ldr r0,=0x020220CC
	;strh r1,[r0,#0x0]
	
	;clear the fight! flag
	ldr r0,=0x020019A4
	mov r4,0
	str r4,[r0]
	
	;return
	ldr r4,=0x08004EFC
	mov r15,r4

.pool
.endfunc

.org 0x08004EF4
	ldr r4,=0x08FFFF70
	mov r15,r4

.pool

.close