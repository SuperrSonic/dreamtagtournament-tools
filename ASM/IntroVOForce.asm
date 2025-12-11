// DTT Character Intro Selector (Force P2 Intro Voice Only)
// When the game plays the introduction voice (Zatch: ikuzo kiyomaro/Zeno: tokoton made butsuku)
// it does so for both P1 and P2, since all voices are on the same group, they override each other
// depending on the timing.

// So, for example: Riya's voice will always play no matter the combination
// because it plays near the end.
// This makes it frustrating as some voices will hardly ever play or even get cut off
// by the other character's voice.

// To solve this I want to offer two options:
// 1. Mimic EA2 by only playing VO for P2.
// 2. Randomly choose the player.

// Going for 1 here, we need to check for both P1 and P2 characters
// because if they match we must skip any added code.

// P1 = 02021F34
// P2 = 02021EF0

// Character IDs
// 01 = Zatch
// 02 = Kanchome
// 03 = Tia
// 04 = Ponygon
// 05 = Wonrei
// 06 = Brago
// 07 = Ted
// 08 = Kido
// 09 = Karudio
// 0A = Zeno
// 0B = Victoream
// 0C = Laila
// 0D = Penny
// 0E = Arth
// 0F = Keith
// 10 = Bari
// 11 = Riya
// 12 = Vulcan

// A simple way to find the value of the sound used by playSound
// is to watch 03007E6C by looping endlessly on the function below

.gba

.open "dtt.gba","DTT_enhancement.gba",0x08000000 


//SaveFlash EQU 0x08001FCD
;.org 0x08000510
;dh 0xE00A     ;branches to the bl that writes the savefile

// These two instructions become redundant, so nop them off
;.org 0x08004A3C
;dw 0x00000000


.org 0x08FFFE80

.area 0x180

.align 4

.func SelectVoiceIntro
	;Replacement of original functions
	push {lr}
	ldrh r2,[r1,#0xe]
	add r2,#0x2
	strh r2,[r1,#0xe]
	
	;Check against matching P1 and P2
	ldr r2,=0x02021F34  ;P1 character
	ldrh r1,[r2]        ;Read into r1
	ldr r2,=0x02021EF0  ;P2 character
	ldrh r3,[r2]        ;Read into r3
	
	cmp r1,r3
	beq playSound
	
	;This is where the fun begins
	cmp r1,1            ;Zatch
	beq goZatch
	cmp r1,2            ;Kanchome
	beq goKanchome
	cmp r1,3            ;Tia
	beq goTia
	cmp r1,4            ;Ponygon
	beq goPonygon
	cmp r1,5            ;Wonrei
	beq goWonrei
	cmp r1,6            ;Brago
	beq goBrago
	cmp r1,7            ;Ted
	beq goTed
	cmp r1,8            ;Kido
	beq goKido
	cmp r1,9            ;Karudio
	beq goKarudio
	cmp r1,0xA          ;Zeno
	beq goZeno
	cmp r1,0xB          ;Victoream
	beq goVictoream
	cmp r1,0xC          ;Laila
	beq goLaila
	cmp r1,0xD          ;Penny
	beq goPenny
	cmp r1,0xE          ;Arth
	beq goArth
	cmp r1,0xF          ;Keith
	beq goKeith
	cmp r1,0x10         ;Bari
	beq goBari
	cmp r1,0x11         ;Riya
	beq goRiya
	cmp r1,0x12         ;Vulcan
	beq goVulcan
	
	;Finish it
	b playSound
	
goZatch:
	ldrh r1,[r0,#0x4]   ;load VO ID
	ldr r3, =0x12F      ;ikuzo Kiyomaro
	b finalCompare

goKanchome:
	ldrh r1,[r0,#0x4]   ;load VO ID
	ldr r3, =0x14A      ;ikuwayo Folgore
	b finalCompare

goTia:
	ldrh r1,[r0,#0x4]   ;load VO ID
	ldr r3, =0x13E      ;ikini kimeruwayo Megumi
	b finalCompare

goPonygon:
	ldrh r1,[r0,#0x4]   ;load VO ID
	ldr r3, =0x15A      ;shudoruk
	cmp r1,r3
	beq skipSound
	ldr r3, =0x156      ;meru meru me
	b finalCompare

goWonrei:
	ldrh r1,[r0,#0x4]   ;load VO ID
	ldr r3, =0x161      ;Ikuzo Lien
	b finalCompare

goBrago:
	ldrh r1,[r0,#0x4]   ;load VO ID
	ldr r3, =0x19C      ;kakugo shiro
	b finalCompare

goTed:
	ldrh r1,[r0,#0x4]   ;load VO ID
	ldr r3, =0x1B5      ;dragner nagur
	cmp r1,r3
	beq skipSound
	ldr r3, =0x1B0      ;Ted grunt
	b finalCompare

goKido:
	ldrh r1,[r0,#0x4]   ;load VO ID
	ldr r3, =0x190      ;kokowatsuyoi
	b finalCompare

goKarudio:
	ldrh r1,[r0,#0x4]   ;load VO ID
	ldr r3, =0x218      ;gidoruk
	cmp r1,r3
	beq skipSound
	ldr r3, =0x214      ;paru paru moon
	b finalCompare

goZeno:
	ldrh r1,[r0,#0x4]   ;load VO ID
	ldr r3, =0x1A6      ;tokoton made butsuku
	b finalCompare

goVictoream:
	ldrh r1,[r0,#0x4]   ;load VO ID
	ldr r3, =0x200      ;fukatsu
	b finalCompare

goLaila:
	ldrh r1,[r0,#0x4]   ;load VO ID
	ldr r3, =0x20A      ;kakugoshinasai
	b finalCompare

goPenny:
	ldrh r1,[r0,#0x4]   ;load VO ID
	ldr r3, =0x1F4      ;ururu ikuwayo
	b finalCompare

goArth:
	ldrh r1,[r0,#0x4]   ;load VO ID
	ldr r3, =0x21F      ;dewamairuzo
	b finalCompare

goKeith:
	ldrh r1,[r0,#0x4]   ;load VO ID
	ldr r3, =0x1BB      ;omaera shimashite aru
	b finalCompare

goBari:
	ldrh r1,[r0,#0x4]   ;load VO ID
	ldr r3, =0x269      ;mooi
	b finalCompare

goRiya:
	ldrh r1,[r0,#0x4]   ;load VO ID
	ldr r3, =0x274      ;aite kinotte aru
	b finalCompare

goVulcan:
	ldrh r1,[r0,#0x4]   ;load VO ID
	ldr r3, =0x258      ;ikuzo vulcan
	b finalCompare


	;ldrh r0,[r0,#0x4] ;keeping original
	;dh 0xF7F8 ;this one is just the first half of the bl

finalCompare:
	cmp r1,r3
	beq skipSound
	b playSound

skipSound:
	ldr r3,=0x08083ac2 ;exit back
	mov r15,r3         ;r15 is the pc, it's an alternate long branch

playSound:
	ldr r3,=0x08083abc ;exit back
	mov r15,r3         ;r15 is the pc, it's an alternate long branch

.pool
.endfunc
.endarea


// Hooking into the VO selector
.org 0x08083ab4
	ldr r3,=0x08FFFE80
	mov r15,r3

.pool

.close