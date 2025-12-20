/* Copyright (c) 2025 Diego A.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <dirent.h>

static const char proj[] = "PROJECT MAKA III";

static const char decIntro[] = "# Dream Tag Tournament Dialogue Layout v1.0\n"
"# This is a text format for editing the DTT script.\n"
"# You can do a variety of things with it.\n"
"# By using \\ you can specify special commands.\n"
"# Here's a list of all commands:\n"
"# \\N        = Linebreak\n"
"# \\LF       = Linefeed? Creates a new textbox for dialogues.\n"
"# \\MULT     = Multiplication sign.\n"
"# \\MUSIC    = 8th note.\n"
"# \\HEART    = Heart icon.\n"
"# \\ARROW    = Arrow icon, UP, DOWN, LEFT, RIGHT.\n"
"# \\END      = Terminates line\n";

static char LANG_ID[7][4] =
{
	"jp",
	"en",
	"es",
	"fr",
	"it",
	"pt",
	"de"
};

//save struct for ref
typedef struct _SaveData
{
	uint32_t checksum;               // This is on the actual file
	union
	{
		uint16_t data[0x40/2];
		struct
		{
			uint32_t magic;          // always SAN3
			uint32_t saveCount;      // increases on every write
			uint8_t difficulty;      // 0-1-2 aka easy, normal, hard
			uint8_t time;            // 0-1-2 aka 60, 90, inf
			uint8_t rounds;          // 2 = 1 round, 0 = 3 rounds, 1 = 5 rounds
			uint8_t unused;          // this would most likely be padding from the settings int
			uint8_t unk_1;           // unk
			uint8_t unk_2;           // unk
			uint8_t unk_3;           // unk
			uint8_t completion;      // Gash Collection completion percentage, 0x64 = 100%
			uint8_t gashColl[0xD];   // each bit represents an item unlocked, except last byte being 0F
			uint8_t unk_rest[0x23];     // unk, character, stage stuff probably
		} ATTRIBUTE_PACKED;
	};
	uint32_t checksum2;              // simple checksum that appears in mem
} SaveData;

// saveCount is used to select the latest save out of the 7 slots
SaveData * dtt_save = NULL;


// The VWF in the game is limited by even values
// this makes a few characters look wrongly spaced
// one of these is the apostrophe, it uses 2 pixels wide, but needs a 3

// A tiring solution is to embed the apostrophe into these characters, as new characters
//'s
//'r
//'m
//'l
//'t
//'v

// As well as accented i needing 3, if I include an alternate i that's centered
// and the regular one when ther's a space or period.

typedef struct _LinesData
{
	unsigned lineptr;
} LinesData;

unsigned ConvertUTF(uint16_t code, bool patched)
{
	// These are the halfwidth values for the decoder.
	
	// Refs
	// https://loc.gov/marc/specifications/specchareacc/JapaneseHiraganaKatakana.html
	// https://www.utf8-chartable.de/unicode-utf8-table.pl
	
	unsigned val = 0;
	
	switch(code) {
		case 0xE000: val = 0x20;
			break;
		
		case 0xE00B: val = 0x3F; // question mark
			break;
		case 0xE00C: val = patched ? 0x44 : 0xE381B2; // Hiragana HI, now D
			//fprintf(fp, "%s%s%s", "\xE3", "\x81", "\xB2"); // this works too
			break;
		case 0xE00D: val = patched ? 0x45 : 0xE38289; // Hiragana RA, now E
			break;
		case 0xE00E: val = patched ? 0x46 : 0xE3818C; // Hiragana GA, now F
			break;
		case 0xE00F: val = patched ? 0x47 : 0xE381AA; // Hiragana NA, now G
			break;
		case 0xE010: val = patched ? 0x4A : 0xE382AB; // Katakana KA, now J
			break;
		case 0xE011: val = patched ? 0x4B : 0xE382BF; // Katakana TA, now K
			break;
		case 0xE012: val = patched ? 0x4C : 0xE3838A; // Katakana NA, now L
			break;
		case 0xE013: val = 0x41; // Native A
			break;
		case 0xE014: val = 0x42; // Native B
			break;
		case 0xE015: val = 0x43; // Native C
			break;
		case 0xE016: val = patched ? 0x4E : 0xE38282; // Hiragana MO, now N
			break;
		case 0xE017: val = patched ? 0x50 : 0xE381A9; // Hiragana DO, now P
			break;
		case 0xE018: val = patched ? 0x51 : 0xE3828B; // Hiragana RU, now Q
			break;
		case 0xE019: val = patched ? 0x53 : 0xE38191; // Hiragana KE, now S
			break;
		case 0xE01A: val = patched ? 0x55 : 0xE381A9; // Hiragana small TU, now U
			break;
		case 0xE01B: val = patched ? 0x56 : 0xE381A6; // Hiragana TE, now V
			break;
		case 0xE01C: val = patched ? 0x57 : 0xE38184; // Hiragana I, now W
			break;
		case 0xE01D: val = 0x52; // Native R
			break;
		case 0xE01E: val = 0x4F; // Native O
			break;
		case 0xE01F: val = 0x4D; // Native M
			break;
		case 0xE020: val = patched ? 0x58 : 0x5D; // ] , now X
			break;
		case 0xE021: val = patched ? 0x59 : 0xE382BD; // Katakana SO, now Y
			break;
		case 0xE022: val = patched ? 0x5A : 0xE38397; // Katakana PU, now Z
			break;
		case 0xE023: val = patched ? 0x61 : 0xE383AA; // Katakana RI, now a
			break;
		case 0xE024: val = patched ? 0x62 : 0x2D; // Minus sign (idk), now b
			break;
		case 0xE025: val = patched ? 0x63 : 0xE38388; // Katakana TO, now c
			break;
		case 0xE026: val = 0x3A; // :
			break;
		case 0xE027: val = 0x25; // %
			break;
		case 0xE028: val = 0x2E; // . 
			break;
		case 0xE029: val = 0x48; // Native H
			break;
		case 0xE02A: val = 0x49; // Native I
			break;
		case 0xE02B: val = 0x54; // Native T
			break;
		case 0xE02C: val = patched ? 0xE280A6 : 0x78; // Some kind of cross sign, now ellipsis
			break;
		case 0xE02D: val = 0xE383BC; // Vowel elongation mark
			break;
		case 0xE02E: val = patched ? 0x64 : 0xE382A2; // Katakana A, now d
			break;
		case 0xE02F: val = patched ? 0x65 : 0xE382AF; // Katakana KU, now e
			break;
		case 0xE030: val = patched ? 0x66 : 0xE382B7; // Katakana SI, now f
			break;
		case 0xE031: val = patched ? 0x67 : 0xE383A8; // Katakana YO, now g
			break;
		case 0xE032: val = patched ? 0x68 : 0xE38090; // Thick bracket left , now h
			break;
		case 0xE033: val = patched ? 0x69 : 0xE38091; // Thick bracket right, now i
			break;
		case 0xE034: val = patched ? 0x6A : 0xE382B9; // Katakana SU, now j
			break;
		case 0xE035: val = patched ? 0x6B : 0xE383A9; // Katakana RA, now k
			break;
		case 0xE036: val = patched ? 0x6C : 0xE382A3; // Katakana small I, now l
			break;
		case 0xE037: val = patched ? 0x6D : 0xE383AB; // Katakana RU, now m
			break;
		case 0xE038: val = patched ? 0x6E : 0xE382B6; // Katakana ZA, now n
			break;
		case 0xE039: val = patched ? 0x6F : 0xE382AF; // Katakana KU, now o
			break;
		case 0xE03A: val = patched ? 0x70 : 0xE382AC; // Katakana GA, now p
			break;
		case 0xE03B: val = patched ? 0x71 : 0xE382AD; // Katakana KI, now q
			break;
		case 0xE03C: val = patched ? 0x72 : 0xE383A4; // Katakana YA, now r
			break;
		case 0xE03D: val = patched ? 0x73 : 0xE38381; // Katakana TI, now s
			break;
		case 0xE03E: val = patched ? 0x74 : 0xE383A1; // Katakana ME, now t
			break;
		case 0xE03F: val = patched ? 0x75 : 0xE38394; // Katakana PI, now u
			break;
		case 0xE040: val = patched ? 0x76 : 0xE38389; // Katakana DO, now v
			break;
		case 0xE041: val = patched ? 0x77 : 0xE383AF; // Katakana WA, now w
			break;
		case 0xE042: val = patched ? 0x78 : 0xE38390; // Katakana BA, now x
			break;
		case 0xE043: val = patched ? 0x79 : 0xE382A4; // Katakana I, now y
			break;
		case 0xE044: val = patched ? 0x7A : 0xE38399; // Katakana BE, now z
			break;
		case 0xE045: val = patched ? 0x27 : 0x74; // idk, it looks like a tiny t
			break;
		case 0xE046: val = patched ? 0x2C : 0x78; // Some tiny x at the top
			break;
		case 0xE047: val = patched ? 0xC3A1 : 0x67; // Some tiny g, now á
			break;
		case 0xE048: val = patched ? 0xC3A9 : 0xEFBDA2; // Top-left bracket, now é
			break;
		case 0xE049: val = patched ? 0xC3AD : 0x20; // SPACE, now í
			break;
		case 0xE04A: val = patched ? 0xC3B3 : 0x20; // SPACE, now ó
			break;
		case 0xE04B: val = patched ? 0xC3BA : 0x20; // SPACE, now ú
			break;
		case 0xE04C: val = patched ? 0xC3BC : 0x20; // SPACE, now ü
			break;
		case 0xE04D: val = patched ? 0xC3B1 : 0x20; // SPACE, now ñ
			break;
		case 0xE04E: val = patched ? 0xC2BF : 0x20; // SPACE, now ¿
			break;
		case 0xE04F: val = patched ? 0xC2A1 : 0x20; // SPACE, now ¡
			break;
		case 0xE050: val = patched ? 0x21 : 0x20; // SPACE, now !
			break;
		case 0xE051: val = patched ? 0xC3A7 : 0x20; // SPACE, now ç
			break;
		
		// Now fullwidth characters
		case 0x8000: val = 0xE38080; // SPACE FW
			break;
		
		// Now fullwidth misplaced katakana
		case 0x8098: val = 0xE383B4; // VU
			break;
		case 0x80E6: val = 0xE383B5; // KA small
			break;
		case 0x80E7: val = 0xE383B6; // KE small
			break;
		
		// Symbols
		case 0x80E8: val = 0xEFBC9F; // ?
			break;
		case 0x80E9: val = 0xEFBC81; // !
			break;
		case 0x80EA: val = 0xE383BC; // - prolonged sound?
			break;
		case 0x80EB: val = 0xE383BC; // - prolonged sound, longer?
			break;
		case 0x80EC: val = 0xE3829B; // voiced sound mark
			break;
		case 0x80ED: val = 0xE3829C; // semi-voiced sound mark
			break;
		case 0x80EE: val = 0xEFBC9C; // <
			break;
		case 0x80EF: val = 0xEFBD9C; // |
			break;
		case 0x80F0: val = 0xEFBCBF; // _
			break;
		case 0x80F1: val = 0xE38081; //0xEFBDA4 = halfwidth comma, JP , FW
			break;
		case 0x80F2: val = 0xEFBDA1; // fullstop
			break;
		case 0x80F3: val = 0xEFBC8C; // ,
			break;
		case 0x80F4: val = 0xEFBC8E; // .
			break;
		case 0x80F5: val = 0xEFBDA5; // katakana middle dot?
			break;
		case 0x80F6: val = 0xEFBC9A; // :
			break;
		case 0x80F7: val = 0xEFBC9B; // ;
			break;
		case 0x80F8: val = 0xE3809D; // some thin quotes closing
			break;
		case 0x80F9: val = 0xE3809E; // some thin quotes start
			break;
		case 0x80FA: printf("Unused code found 0x%X, report issue!\n", code); //val = 0xE3; // No idea what this is
			break;
		case 0x80FB: val = 0xEFBCBE; // ^
			break;
		case 0x80FC: val = 0xEFBC8D; // -
			break;
		case 0x80FD: val = 0xE383BD; // Katakana iteration mark
			break;
		case 0x80FE: val = 0xE383BE; // Katakana voiced iteration mark
			break;
		case 0x80FF: val = 0xE3829D; // Hiragana iteration mark
			break;
		case 0x8100: val = 0xE3829E; // Hiragana voiced iteration mark
			break;
		case 0x8101: val = 0xE38083; // Not sure which quotation symbol this is
			break;
		case 0x8102: val = 0xE4BB9D; // Kanji repetition mark
			break;
		case 0x8103: val = 0xE38085; // Ideographic iteration mark
			break;
		case 0x8104: val = 0xE38086; // diagonal with squiggly
			break;
		case 0x8105: val = 0xE38087; // diagonal with squiggly
			break;
		case 0x8106: val = 0xEFBC8D; // another hyphen-like design
			break;
		case 0x8107: val = 0xEFBC8F; // / SOLIDUS
			break;
		case 0x8108: val = 0xEFBCBC; // inverted /
			break;
		case 0x8109: val = 0xEFBD9E; // ~
			break;
		case 0x810A:
		case 0x810B:
		case 0x810C:
			printf("Unused chr 0x%X, report issue!\n", code); //val = 0x; // Two vertical lines
			break;
		case 0x810D: val = 0xEFBC87; // apostrophe right
			break;
		case 0x810E: val = 0xEFBC87; // apostrophe left TODO: find the correct values
			break;
		case 0x810F: val = 0xEFBC87; // idk, check again
			break;
		case 0x8110:
			printf("Script has unusual SPACE character! 0x%X\n", code);
			val = 0xE38080; // another space
			break;
		case 0x8111: val = 0x28; // ( TODO: use the fw value
			break;
		case 0x8112: val = 0x29; // ) TODO: use the fw value
			break;
		case 0x8113:
		case 0x8114:
		case 0x8115:
		case 0x8116:
		case 0x8117:
			printf("Script has unusual SPACE character! 0x%X\n", code);
			val = 0xE38080; // another space
			break;
		case 0x8118: val = 0xEFBD9D; // }
			break;
		case 0x8119: val = 0xE28CA9; // ( variant
			break;
		case 0x811A: val = 0xE28CAA; // ) variant
			break;
		case 0x811B: val = 0xEFBD9F; // (( white
			break;
		case 0x811C: val = 0xEFBDA0; // )) white
			break;
		case 0x811D: val = 0xEFBDA2; // hw corner bracket top
			break;
		case 0x811E: val = 0xEFBDA3; // hw corner bracket bot
			break;
		case 0x811F: val = 0xE3808E; // corner bracket white top
			break;
		case 0x8120: val = 0xE3808F; // corner bracket white bot
			break;
		case 0x8121: val = 0xE38090; // JP bracket left
			break;
		case 0x8122: val = 0xE38091; // JP bracket right
			break;
		case 0x8123: val = 0xEFBC8B; // +
			break;
		case 0x8124: val = 0xEFBC8D; // -
			break;
		case 0x8125: val = 0xC2B1; // +-
			break;
		case 0x8126: val = 0xC397; // mult sign
			break;
		case 0x8127: val = 0xC3B7; // div sign
			break;
		case 0x8128: val = 0xEFBC9D; // =
			break;
		case 0x8129: val = 0xE289A0; // crossed =
			break;
		case 0x812A: val = 0xEFBC9E; // >
			break;
		case 0x812B: val = 0xE289A6; // <=
			break;
		case 0x812C: val = 0xE289A7; // >=
			break;
		case 0x812D: val = 0xE2889E; // Inf
			break;
		case 0x812E: printf("Unusual chr! 0x%X Using fullstop.\n"); val = 0xEFBC8E; // three dots in pyramid position
			break;
		case 0x812F: val = 0xE29982; // Female
			break;
		case 0x8130: val = 0xE29980; // Male
			break;
		case 0x8131: val = 0xE3829C; // Handakuten circle
			break;
		case 0x8132: val = 0xCABC; // apostrophe?
			break;
		case 0x8133: val = 0xCBAE; // "
			break;
		case 0x8134: val = 0xE28483; // box? idk
			break;
		case 0x8135: val = 0xEFBFA5; // Yen
			break;
		case 0x8136: val = 0xEFBC84; // MONEY MONEY MONEY
			break;
		case 0x8137: val = 0xC2A2; // cent
			break;
		case 0x8138: val = 0xEFBFA1; // Euro
			break;
		case 0x8139: val = 0xEFBC85; // %
			break;
		case 0x813A: val = 0xEFBC83; // #
			break;
		case 0x813B: val = 0xEFBC86; // &
			break;
		case 0x813C: val = 0xEFBC8A; // *
			break;
		case 0x813D: val = 0xEFBCA0; // @
			break;
		case 0x813E: val = 0xC2A7; // S with squiggly
			break;
		case 0x813F: val = 0xE29CB0; // transparent star
			break;
		case 0x8140: val = 0xE29886; // white star
			break;
		case 0x8141: val = 0xE38087; // seikai, Madoka Kyono: maru!
			break;
		case 0x8142: val = 0xE2978B; // small circle
			break;
		case 0x8143: val = 0xE28CBE; // box? idk
			break;
		case 0x8144: val = 0xE29786; // transparent diamond
			break;
		case 0x8145: val = 0xE29787; // black diamond
			break;
		case 0x8146: val = 0xE296A0; // transparent square?
			break;
		case 0x8147: val = 0xE296A1; // square?
			break;
		case 0x8148: val = 0xE296B2; // transparent triangle
			break;
		case 0x8149: val = 0xE296B3; // black triangle
			break;
		case 0x814A: val = 0xE296BC; // downwards transparent triangle
			break;
		case 0x814B: val = 0xE296BD; // downwards triangle
			break;
		case 0x814C: val = 0xE280BB; // JP kome
			break;
		case 0x814D: val = 0xE38092; // JP yuubin
			break;
		case 0x814E: val = 0xE28692; // arrow pointing right
			break;
		case 0x814F: val = 0xE28690; // arrow pointing left
			break;
		case 0x8150: val = 0xE28691; // arrow pointing up
			break;
		case 0x8151: val = 0xE28693; // arrow pointing down
			break;
		case 0x8152: val = 0xE38093; // = thicker
			break;
		// I can't believe there's no symbol for music!
		
		// Kanji time, baby!
		case 0x8153: val = 0xE6B885; // Kiyo
			break;
		case 0x8154: val = 0xE9BABF; // maro
			break;
		case 0x8155: val = 0xE681B5; // Megumi
			break;
		case 0x8156: val = 0xE58D9A; // ?
			break;
	}
	
	// In case these symbols or blank spaces show up, use kome
//	if(code > 0x84BD && code < 0x84CD)
//		val = 0xE280BB;
	
#if 1
	// Numbers HW
	if(code > 0xE000 && code < 0xE00B) {
		uint16_t i = 0;
		val = 0x2F;
		for(i = 0; i < (code - 0xE000); ++i) {
			++val;
		}
	}
	// Numbers FW
	else if(code > 0x8000 && code < 0x800B) {
		uint16_t i = 0;
		val = 0xEFBC8F;
		for(i = 0; i < (code - 0x8000); ++i) {
			++val;
		}
	}
	// Letters FW
	else if(code > 0x800A && code < 0x8025) {
		uint16_t i = 0;
		val = 0xEFBCA0;
		for(i = 0; i < (code - 0x800A); ++i) {
			++val;
		}
	}
	// Lowercase letters FW
	else if(code > 0x8024 && code < 0x803F) {
		uint16_t i = 0;
		val = 0xEFBD80;
		for(i = 0; i < (code - 0x8024); ++i) {
			++val;
		}
	}
	// Hiragana part 1
	else if(code > 0x803E && code < 0x807E) {
		uint16_t i = 0;
		val = 0xE38180;
		for(i = 0; i < (code - 0x803E); ++i) {
			//printf("SHOW: 0x%X\n", val);
			
			++val;
		}
	}
	// Hiragana part 2
	else if(code > 0x807D && code < 0x8092) {
		uint16_t i = 0;
		val = 0xE3827F;
		for(i = 0; i < (code - 0x807D); ++i) {
			++val;
		}
	}
	// Katakana p1 misplaced VU
	else if(code > 0x8091 && code < 0x8098) {
		uint16_t i = 0;
		val = 0xE382A0;
		for(i = 0; i < (code - 0x8091); ++i)
			++val;
	}
	// Katakana p2
	else if(code > 0x8098 && code < 0x80B2) {
		uint16_t i = 0;
		val = 0xE382A6;
		for(i = 0; i < (code - 0x8098); ++i)
			++val;
	}
	// Katakana p3
	else if(code > 0x80B1 && code < 0x80E6) {
		uint16_t i = 0;
		val = 0xE3837F;
		for(i = 0; i < (code - 0x80B1); ++i)
			++val;
	}
#endif
	
	return val;
}

unsigned ConvertCharJP(uint8_t text1, uint8_t text2, uint8_t text3)
{
	unsigned val = 0;
	unsigned chr = text3 | text2 << 8 | text1 << 16;
	
	//printf("SHOW24: 0x%X\n", chr);
	
	switch (chr) {
		
		case 0xE280A6: val = 0xE02C; // ...
			break;
		case 0xE383BC: val = 0xE02D; // Vowel elongation
			break;
		
		// It takes too much time to write all the JP characters
		case 0xE38080: val = 0x8000; // SPACE FW
			break;
		
		case 0xEFBC9F: val = 0x80E8; // ? FW
			break;
		case 0xEFBC81: val = 0x80E9; // ! FW
			break;
		case 0xE3829B: val = 0x80EC; // " FW
			break;
		case 0xEFBC9C: val = 0x80EE; // < FW
			break;
		case 0xE38081: val = 0x80F1; // JP ,? FW
			break;
		case 0xEFBDA4: val = 0x80F1; // JP ,? FW
			break;
		case 0xE38082: val = 0x80F2; // JP . FW
			break;
		case 0xEFBC8E: val = 0x80F4; // . FW
			break;
		case 0xEFBC9A: val = 0x80F6; // : FW
			break;
		case 0xEFBC9B: val = 0x80F7; // ; FW
			break;
		case 0xE3809E: val = 0x80F8; // right " FW
			break;
		case 0xE3809D: val = 0x80F9; // left " FW
			break;
		case 0xEFBFA5: val = 0x8135; // Yen FW
			break;
		case 0xE38087: val = 0x8141; // Seikai FW
			break;
		// Honestly, I doubt these symbols are used in the game so
		// only the ones I thought might appear are here.
		
		case 0xEFBC85: val = 0x8139; // % FW (Has HW ver.)
			break;
		
		
	//	case 0xE38181: val = 0x803F; // Hiragana A small
	//		break;
	//	case 0xE38182: val = 0x8040; // Hiragana A
	//		break;
		
	}
	
	// These cover numbers and letters in FW
	if(chr > 0xEFBC8F && chr < 0xEFBC9A) {
		uint16_t i = 0;
		val = 0x8000;
		for(i = 0; i < (chr - 0xEFBC8F); ++i) {
			++val;
		}
	}
	else if(chr > 0xEFBCA0 && chr < 0xEFBCBB) {
		uint16_t i = 0;
		val = 0x800A;
		for(i = 0; i < (chr - 0xEFBCA0); ++i) {
			++val;
		}
	}
	else if(chr > 0xEFBD80 && chr < 0xEFBD9B) {
		uint16_t i = 0;
		val = 0x8024;
		for(i = 0; i < (chr - 0xEFBD80); ++i) {
			++val;
		}
	}
	// Hiragana part 1
	else if(chr > 0xE38180 && chr < 0xE381C0) {
		uint16_t i = 0;
		val = 0x803E;
		for(i = 0; i < (chr - 0xE38180); ++i)
			++val;
	}
	// Hiragana part 2
	else if(chr > 0xE3827F && chr < 0xE38294) {
		uint16_t i = 0;
		val = 0x807D;
		for(i = 0; i < (chr - 0xE3827F); ++i) {
			++val;
		}
	}
	// Katakana p1 misplaced VU
	else if(chr > 0xE382A0 && chr < 0xE382A7) {
		uint16_t i = 0;
		val = 0x8091;
		for(i = 0; i < (chr - 0xE382A0); ++i)
			++val;
	}
	// Katakana p2
	else if(chr > 0xE382A6 && chr < 0xE382C0) {
		uint16_t i = 0;
		val = 0x8098;
		for(i = 0; i < (chr - 0xE382A6); ++i)
			++val;
	}
	// Katakana p3
	else if(chr > 0xE3837F && chr < 0xE383B4) {
		uint16_t i = 0;
		val = 0x80B1;
		for(i = 0; i < (chr - 0xE3837F); ++i)
			++val;
	}
	
	return val;
}

unsigned ConvertCharSymb(uint8_t text1, uint8_t text2)
{
	unsigned val = 0;
	uint16_t chr = text2 | text1 << 8;
	
	//printf("SHOW16: 0x%X\n", chr);
	
	switch (chr) {
		case 0xC3A1: val = 0xE047; // á
			break;
		case 0xC3A9: val = 0xE048; // é
			break;
		case 0xC3AD: val = 0xE049; // í
			break;
		case 0xC3B3: val = 0xE04A; // ó
			break;
		case 0xC3BA: val = 0xE04B; // ú
			break;
		case 0xC3BC: val = 0xE04C; // ü
			break;
		case 0xC3B1: val = 0xE04D; // ñ
			break;
		case 0xC2BF: val = 0xE04E; // ¿
			break;
		case 0xC2A1: val = 0xE04F; // ¡
			break;
		case 0xC3A7: val = 0xE051; // ç
			break;
	}
	
	return val;
}

unsigned ConvertChar(char text)
{
  unsigned val = 0;

  switch (text) {


   case '_': val = 0x8163; //use underscore for ellipsis
      break;
   case '”': val = 0x8168; //alt 0148
      break;
   case '“': val = 0x8167; //alt 0147
      break;
   case '’': val = 0x8166; //alt 0146
      break;
   case '<': val = 0x8183;
      break;
   case '>': val = 0x8184; //requires quotes
      break;
   case ':': val = 0x8146;
      break;
   case ';': val = 0x8147;
      break;
   case '/': val = 0x815E;
      break;
   case '\\': val = 0x815E; // not in DTT, using solidus
      break;
   case '~': val = 0x8160;
      break;
   case '(': val = 0x8169;
      break;
   case ')': val = 0x816A;
      break;
   case '[': val = 0x816D; // JP style
      break;
   case ']': val = 0x816E; // JP style
      break;
   case '+': val = 0x817B;
      break;
   case '-': val = 0x817C;
      break;
   //case '=': val = 0x8181; // not in DTT
      //break;
   case '\"': val = 0x8168; //quotes fullwidth
      break;
   case '&': val = 0x8195; //needs to be escaped \& or in quotes depending on situation
      break;
   case '*': val = 0x8196; //needs special care
      break;
   case '%': val = 0x8193;
      break;
   case ',': val = 0x8143;
      break;
   case '.': val = 0x8144;
      break;
   case '\'': val = 0x8166;
      break;
   case '…': val = 0x8163;
      break;
   case '#': val = 0x8194; // In DTT it's an angry/frustrated-like vein sign
      break;

   // 0x81A6 = kome/reference mark/also placeholder in DTT

   case ' ': val = 0x8140; //space used for alignment, need a 3px version to match MinishCap
      break;
   case '0': val = 0x824F;
      break;
   case '1': val = 0x8250;
      break;
   case '2': val = 0x8251;
      break;
   case '3': val = 0x8252;
      break;
   case '4': val = 0x8253;
      break;
   case '5': val = 0x8254;
      break;
   case '6': val = 0x8255;
      break;
   case '7': val = 0x8256;
      break;
   case '8': val = 0x8257;
      break;
   case '9': val = 0x8258;
      break;
   case '@': val = 0x8197;
      break;
   case 'A': val = 0x8260;
      break;
   case 'B': val = 0x8261;
      break;
   case 'C': val = 0x8262;
      break;
   case 'D': val = 0x8263;
      break;
   case 'E': val = 0x8264;
      break;
   case 'F': val = 0x8265;
      break;
   case 'G': val = 0x8266;
      break;
   case 'H': val = 0x8267;
      break;
   case 'I': val = 0x8268;
      break;
   case 'J': val = 0x8269;
      break;
   case 'K': val = 0x826A;
      break;
   case 'L': val = 0x826B;
      break;
   case 'M': val = 0x826C;
      break;
   case 'N': val = 0x826D;
      break;
   case 'O': val = 0x826E;
      break;
   case 'P': val = 0x826F;
      break;
   case 'Q': val = 0x8270;
      break;
   case 'R': val = 0x8271;
      break;
   case 'S': val = 0x8272;
      break;
   case 'T': val = 0x8273;
      break;
   case 'U': val = 0x8274;
      break;
   case 'V': val = 0x8275;
      break;
   case 'W': val = 0x8276;
      break;
   case 'X': val = 0x8277;
      break;
   case 'Y': val = 0x8278;
      break;
   case 'Z': val = 0x8279;
      break;
   case 'a': val = 0x829F;
      break;
   case 'b': val = 0x82A0;
      break;
   case 'c': val = 0x82A1;
      break;
   case 'd': val = 0x82A2;
      break;
   case 'e': val = 0x82A3;
      break;
   case 'f': val = 0x82A4;
      break;
   case 'g': val = 0x82A5;
      break;
   case 'h': val = 0x82A6;
      break;
   case 'i': val = 0x82A7;
      break;
   case 'j': val = 0x82A8;
      break;
   case 'k': val = 0x82A9;
      break;
   case 'l': val = 0x82AA;
      break;
   case 'm': val = 0x82AB;
      break;
   case 'n': val = 0x82AC;
      break;
   case 'o': val = 0x82AD;
      break;
   case 'p': val = 0x82AE;
      break;
   case 'q': val = 0x82AF;
      break;
   case 'r': val = 0x82B0;
      break;
   case 's': val = 0x82B1;
      break;
   case 't': val = 0x82B2;
      break;
   case 'u': val = 0x82B3;
      break;
   case 'v': val = 0x82B4;
      break;
   case 'w': val = 0x82B5;
      break;
   case 'x': val = 0x82B6;
      break;
   case 'y': val = 0x82B7;
      break;
   case 'z': val = 0x82B8;
      break;
   case 'ñ': val = 0x82B9;
      break;
   case 'á': val = 0x82BA;
      break;
   case 'é': val = 0x82BB;
      break;
   case 'í': val = 0x82BC;
      break;
   case 'ó': val = 0x82BD;
      break;
   case 'ú': val = 0x82BE;
      break;
   case 'ü': val = 0x82BF;
      break;
   case 'ç': val = 0x82C0;
      break;
   case '¿': val = 0x82C1;
      break;
   case '?': val = 0x8148;
      break;
   case '¡': val = 0x82C2;
      break;
   case '!': val = 0x8149;
      break;
  }

  return val;
}

// This contains all 214 PCM sizes used by voice samples in ROM order (not Sound Test order)
static uint16_t arr_sizes[214] = {
	0x2C30, 0x2B30, 0x3E90, 0x30B0, 0x6822, 0x1E10, 0x26D0, 0x4200, 0x3160, 0x3010, 0x15C0, 0x1D10,
	0x1F00, 0x2198, 0x2570, 0x2480, 0x36A0, 0x28C0, 0x4418, 0x3060, 0x2084, 0x2750, 0x12BC, 0x152C,
	0x24D0, 0x2AA8, 0x1E9C, 0x3B00, 0x3640, 0x1577, 0x2CCA, 0x4B10, 0x3010, 0x1214, 0x1268, 0x1098,
	0x1300, 0x1790, 0x1FA8, 0x1E2B, 0x2380, 0x1730, 0x3C44, 0x37F0, 0x4040, 0x1B84, 0x1800, 0x22A0,
	0x3948, 0x2554, 0x2B98, 0x2800, 0x2790, 0x1EBC, 0x43A0, 0x22BC, 0x17B4, 0x2408, 0x1238, 0x1FB0,
	0x2490, 0x2148, 0x36E8, 0x12DC, 0x2A48, 0x3880, 0x39E8, 0x44E8, 0x18B5, 0x1070, 0x1670, 0x2A74,
	0x1F70, 0x1C00, 0x0F64, 0x3070, 0x0EF3, 0x19D8, 0x47D0, 0x11F8, 0x15D0, 0x12F0, 0x26F0, 0x27D0,
	0x2EC0, 0x32A0, 0x2580, 0x3900, 0x15EF, 0x1D08, 0x09F0, 0x0E59, 0x1750, 0x172A, 0x12EE, 0x24F0,
	0x0E9B, 0x1474, 0x2F20, 0x26AC, 0x2290, 0x1A28, 0x26C4, 0x1FE0, 0x2260, 0x25C8, 0x2630, 0x3210,
	0x3E10, 0x3881, 0x3210, 0x26A0, 0x22F0, 0x2F38, 0x2FF8, 0x31C0, 0x2796, 0x1D30, 0x325C, 0x4778,
	0x1740, 0x3AAE, 0x4A48, 0x1310, 0x2308, 0x1738, 0x1968, 0x2650, 0x1F90, 0x0D1A, 0x21A8, 0x2C21,
	0x1F20, 0x43A8, 0x2064, 0x3C54, 0x1AB0, 0x2460, 0x1688, 0x32E8, 0x23B0, 0x2A30, 0x4170, 0x1850,
	0x1564, 0x20CC, 0x1C00, 0x2DA0, 0x2D60, 0x1DDC, 0x26B8, 0x3330, 0x32A8, 0x26C8, 0x16BE, 0x23A0,
	0x12D0, 0x17B4, 0x2BE0, 0x3748, 0x22E0, 0x2140, 0x40FC, 0x4360, 0x1E38, 0x1CE8, 0x1B88, 0x2104,
	0x3138, 0x1024, 0x2EA0, 0x26D0, 0x2E48, 0x3818, 0x224F, 0x2EA0, 0x10C8, 0x3A70, 0x2374, 0x1F70,
	0x1800, 0x2618, 0x2B50, 0x28F0, 0x30E0, 0x4631, 0x28B0, 0x235F, 0x3030, 0x41FA, 0x2B99, 0x27A0,
	0x1D60, 0x133F, 0x1B6F, 0x1B60, 0x262A, 0x1F32, 0x2320, 0x1D04, 0x29F8, 0x31F0, 0x1DA8, 0x1238,
	0x18A8, 0x24D0, 0x2FA8, 0x2DB0, 0x2134, 0x42C0, 0x177C, 0x1180, 0x0EFC, 0x2088
};

static uint32_t arr_songs[33] = {
	0xBCA954, 0xBCA964, 0xBCA96C, 0xBCA974, 0xBCA97C, 0xBCA9A4, 0xBCA9CC, 0xBCA9AC,
	0xBCA9DC, 0xBCA9B4, 0xBCA9BC, 0xBCA9C4, 0xBCA9D4, 0xBCA9E4, 0xBCA9EC, 0xBCA9F4,
	0xBCAA0C, 0xBCA9FC, 0xBCAA04, 0xBCAA44, 0xBCAA4C, 0xBCAA54, 0xBCAA94, 0xBCAA9C,
	0xBCAAA4, 0xBCAAAC, 0xBCAAB4, 0xBCAAE4, 0xBCAAEC, 0xBCAAF4, 0xBCAAFC, 0xBCAB04,
	0xBCAB0C
};

static uint8_t dontPauseStream_bin[48] = {
	0x06, 0x49, 0x07, 0x48, 0x00, 0x88, 0x40, 0x00, 0x40, 0x18, 0x00, 0x88, 0x05, 0x4A, 0x12, 0x78,
	0x08, 0x2A, 0x01, 0xDC, 0x04, 0x49, 0x8F, 0x46, 0x04, 0x49, 0x8F, 0x46, 0x00, 0x1F, 0x02, 0x02,
	0x48, 0x20, 0x02, 0x02, 0xB3, 0x19, 0x00, 0x02, 0x48, 0x4A, 0x00, 0x08, 0x4C, 0x4A, 0x00, 0x08
};

static uint8_t forceVoiceIntro_bin[0x150] = {
	0x00, 0xB5, 0xCA, 0x89, 0x02, 0x32, 0xCA, 0x81, 0x38, 0x4A, 0x11, 0x88, 0x38, 0x4A, 0x13, 0x88,
	0x99, 0x42, 0x68, 0xD0, 0x01, 0x29, 0x22, 0xD0, 0x02, 0x29, 0x23, 0xD0, 0x03, 0x29, 0x24, 0xD0,
	0x04, 0x29, 0x25, 0xD0, 0x05, 0x29, 0x29, 0xD0, 0x06, 0x29, 0x2A, 0xD0, 0x07, 0x29, 0x2B, 0xD0,
	0x08, 0x29, 0x2F, 0xD0, 0x09, 0x29, 0x30, 0xD0, 0x0A, 0x29, 0x34, 0xD0, 0x0B, 0x29, 0x35, 0xD0,
	0x0C, 0x29, 0x36, 0xD0, 0x0D, 0x29, 0x37, 0xD0, 0x0E, 0x29, 0x38, 0xD0, 0x0F, 0x29, 0x39, 0xD0,
	0x10, 0x29, 0x3A, 0xD0, 0x11, 0x29, 0x3B, 0xD0, 0x12, 0x29, 0x3C, 0xD0, 0x43, 0xE0, 0x81, 0x88,
	0x24, 0x4B, 0x3B, 0xE0, 0x81, 0x88, 0x24, 0x4B, 0x38, 0xE0, 0x81, 0x88, 0x23, 0x4B, 0x35, 0xE0,
	0x81, 0x88, 0x23, 0x4B, 0x99, 0x42, 0x34, 0xD0, 0x22, 0x4B, 0x2F, 0xE0, 0x81, 0x88, 0x22, 0x4B,
	0x2C, 0xE0, 0x81, 0x88, 0x21, 0x4B, 0x29, 0xE0, 0x81, 0x88, 0x21, 0x4B, 0x99, 0x42, 0x28, 0xD0,
	0x20, 0x4B, 0x23, 0xE0, 0x81, 0x88, 0x20, 0x4B, 0x20, 0xE0, 0x81, 0x88, 0x1F, 0x4B, 0x99, 0x42,
	0x1F, 0xD0, 0x1F, 0x4B, 0x1A, 0xE0, 0x81, 0x88, 0x1E, 0x4B, 0x17, 0xE0, 0x81, 0x88, 0x1E, 0x4B,
	0x14, 0xE0, 0x81, 0x88, 0x1D, 0x4B, 0x11, 0xE0, 0x81, 0x88, 0x1D, 0x4B, 0x0E, 0xE0, 0x81, 0x88,
	0x1C, 0x4B, 0x0B, 0xE0, 0x81, 0x88, 0x1C, 0x4B, 0x08, 0xE0, 0x81, 0x88, 0x1B, 0x4B, 0x05, 0xE0,
	0x81, 0x88, 0x1B, 0x4B, 0x02, 0xE0, 0x81, 0x88, 0x1A, 0x4B, 0xFF, 0xE7, 0x99, 0x42, 0x00, 0xD0,
	0x01, 0xE0, 0x19, 0x4B, 0x9F, 0x46, 0x19, 0x4B, 0x9F, 0x46, 0x00, 0x00, 0x34, 0x1F, 0x02, 0x02,
	0xF0, 0x1E, 0x02, 0x02, 0x2F, 0x01, 0x00, 0x00, 0x4A, 0x01, 0x00, 0x00, 0x3E, 0x01, 0x00, 0x00,
	0x5A, 0x01, 0x00, 0x00, 0x56, 0x01, 0x00, 0x00, 0x61, 0x01, 0x00, 0x00, 0x9C, 0x01, 0x00, 0x00,
	0xB5, 0x01, 0x00, 0x00, 0xB0, 0x01, 0x00, 0x00, 0x90, 0x01, 0x00, 0x00, 0x18, 0x02, 0x00, 0x00,
	0x14, 0x02, 0x00, 0x00, 0xA6, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x0A, 0x02, 0x00, 0x00,
	0xF4, 0x01, 0x00, 0x00, 0x1F, 0x02, 0x00, 0x00, 0xBB, 0x01, 0x00, 0x00, 0x69, 0x02, 0x00, 0x00,
	0x74, 0x02, 0x00, 0x00, 0x58, 0x02, 0x00, 0x00, 0xC2, 0x3A, 0x08, 0x08, 0xBC, 0x3A, 0x08, 0x08
};

static uint8_t dtt_badge_bin[0x90] = {
	0xB8, 0x60, 0xFF, 0x20, 0xF8, 0x76, 0x30, 0x1C, 0x19, 0x49, 0x0E, 0x78, 0x64, 0x2E, 0x2D, 0xDB,
	0x18, 0x49, 0x0E, 0x78, 0x18, 0x4B, 0x98, 0x42, 0x09, 0xD0, 0x18, 0x4B, 0x98, 0x42, 0x0B, 0xD0,
	0x17, 0x4B, 0x98, 0x42, 0x0D, 0xD0, 0x17, 0x4B, 0x98, 0x42, 0x0F, 0xD0, 0x1B, 0xE0, 0x01, 0x36,
	0x01, 0x2E, 0x18, 0xD1, 0x0E, 0x70, 0x19, 0xE0, 0x01, 0x36, 0x02, 0x2E, 0x13, 0xD1, 0x0E, 0x70,
	0x14, 0xE0, 0x01, 0x36, 0x03, 0x2E, 0x0E, 0xD1, 0x0E, 0x70, 0x0F, 0xE0, 0x01, 0x36, 0x04, 0x2E,
	0x09, 0xD1, 0x00, 0x26, 0x0E, 0x70, 0x67, 0x20, 0xFF, 0xE7, 0x0B, 0x4B, 0x1E, 0x78, 0x10, 0x22,
	0x16, 0x43, 0x1E, 0x70, 0xFF, 0xE7, 0x00, 0x26, 0x0E, 0x70, 0xFF, 0xE7, 0x07, 0x49, 0x8F, 0x46,
	0x5F, 0x20, 0x02, 0x02, 0xB0, 0x1B, 0x00, 0x03, 0x2E, 0x01, 0x00, 0x00, 0x11, 0x02, 0x00, 0x00,
	0x50, 0x01, 0x00, 0x00, 0x35, 0x01, 0x00, 0x00, 0x6C, 0x20, 0x02, 0x02, 0x20, 0x57, 0x01, 0x08
};

//Add 0x10 to get the tilemaps
static uint32_t arr_cred_ptr[28] = {
	0x211EFC, 0x212230, 0x2126F4, 0x212BCC, 0x2130F8, 0x213594, 0x213A28, 0x213E6C,
	0x2141B8, 0x214450, 0x214738, 0x214974, 0x214CE8, 0x21511C, 0x21553C, 0x215930,
	0x215C40, 0x216008, 0x2162C0, 0x216580, 0x2169E4, 0x216EC0, 0x217374, 0x21762C,
	0x217A80, 0x217CF4, 0x217F70, 0x218268
};

//sizes for tilemaps and tilesets allows optimizing space
static uint16_t arr_cred_size[28] = {
	0x0107, 0x01EC, 0x031C, 0x0319, 0x0358, 0x02E1, 0x02F5, 0x02AF,
	0x01FB, 0x017C, 0x01AE, 0x013B, 0x021F, 0x02AD, 0x02A1, 0x027D,
	0x01E9, 0x0253, 0x018F, 0x018B, 0x02C5, 0x0326, 0x030E, 0x0191,
	0x02BB, 0x0166, 0x0153, 0x01D3
};

static uint16_t arr_cred_map_size[28] = {
	0x00DA, 0x012B, 0x018A, 0x019D, 0x01B8, 0x019C, 0x0180, 0x0176,
	0x0133, 0x0100, 0x011C, 0x00E2, 0x0137, 0x0167, 0x015D, 0x0158,
	0x0107, 0x0156, 0x0109, 0x0117, 0x0180, 0x0196, 0x0186, 0x0105,
	0x017B, 0x00ED, 0x010A, 0x0107
};

static uint8_t msgEOL[0x20] = {
	0x46, 0x75, 0x6E, 0x20, 0x69, 0x73, 0x20, 0x69, 0x6E, 0x66,
	0x69, 0x6E, 0x69, 0x74, 0x65, 0x2E, 0x44, 0x69, 0x65, 0x67,
	0x6F, 0x20,	0x41, 0x2E, 0x20, 0x32, 0x30, 0x32, 0x35, 0xFF,
	0xFF, 0xFF
};

void tilemapIndexer(int datSize, char* path)
{
	//lack of knowledge on grit or maybe it just doesn't do this
	//but I need flat tilemaps to be set to a specific palette
	
	uint8_t* tilemapDat = malloc(datSize);
	FILE* fp = NULL;
	fp = fopen(path, "rb");
	if(fp){
		fread(tilemapDat, 1, datSize, fp);
		fclose(fp);
		int i = 0;
		for(i=1;i<datSize;i+=2) {
			tilemapDat[i] &= 0xF;
			tilemapDat[i] |= 0xF0; // use palette 15
		}
		//printf("Show me: 0x%02X\n", tilemapDat[1]);
		
		fp = fopen("credX_map.bin", "wb");
		if(fp) {
			fwrite(tilemapDat, 1, datSize, fp);
			fclose(fp);
		}
	}
	
	if(tilemapDat)
		free(tilemapDat);
}

void showMSG(int type, int val, int addr)
{
	if(type == 1)
		printf("Replaced GFX: Gash Collection item no.%03d at 0x%X\n", val, addr);
	else if(type == 2)
		printf("Replaced GFX: Gash Collection req. no.%02d/24 at 0x%X\n", val, addr);
	else if(type == 3)
		printf("Replaced GFX: Generic #%d at 0x%06X ", val, addr);
	else if(type == 4)
		printf("Replaced GFX: CREDITS #%d at 0x%06X ", val, addr);
	else if(type == 5)
		printf("Replaced GFX: CREDITS TILEMAP #%d at 0x%06X ", val, addr);
}

void GenericPatch(uint8_t* romBuf, int langID, char* folder)
{
	DIR *dir;
	struct dirent *entry = NULL;
	int addr, size, ptr, count = 0;
	int magic = 0x33584647;
	char path[64] = {0};
	char full_path[128] = {0};
	FILE* fp = NULL;
	sprintf(path, "GFX/%s/%s/", folder, LANG_ID[langID]);
	
	//printf("Show me: %s\n", path);
	
	dir = opendir(path);
	if (dir==NULL)
		return;
	
	printf("\n");
	
	while ((entry = readdir(dir)))
	{
		size_t length = strlen(entry->d_name);
		if (length > 4 && stricmp(entry->d_name+length-4, ".SPR")==0)
		{
			strcpy(path, entry->d_name);
			count++;
			
			sprintf(full_path, "GFX/%s/%s/%s", folder, LANG_ID[langID], path);
			fp = fopen(full_path, "rb");
			if(fp != NULL) {
				int header = 0;
				fread(&header, 1, 4, fp);
				if(header == magic) {
					// get addr
					fread(&addr, 1, 4, fp);
					if(addr) {
						// fix le
						addr = (addr & 0xFF) << 24 |
							((addr >> 8) & 0xFF) << 16 |
								((addr >> 16) & 0xFF) << 8 |
									((addr >> 24) & 0xFF);
						//printf("SHOW ADDRESS: 0x%X\n", addr);
					}
					
					// get size
					fread(&size, 1, 4, fp);
					if(size) {
						// fix le
						size = (size & 0xFF) << 24 |
							((size >> 8) & 0xFF) << 16 |
								((size >> 16) & 0xFF) << 8 |
									((size >> 24) & 0xFF);
						//printf("SHOW SIZE: 0x%X\n", size);
					}
					
					// get pointer
					fread(&ptr, 1, 4, fp);
					if(ptr) {
						// fix le
						ptr = (ptr & 0xFF) << 24 |
							((ptr >> 8) & 0xFF) << 16 |
								((ptr >> 16) & 0xFF) << 8 |
									((ptr >> 24) & 0xFF);
					}
					
					// patch it
					if(addr) {
						if(size == 0) // idk
							size = 0x800;
						fseek(fp, 0x10, SEEK_SET);
						fread(&romBuf[addr], 1, size, fp);
						showMSG(3, count, addr);
						
						printf(" NAME: %s\n", entry->d_name);
						
						if(ptr) {
							addr |= 8 << 24;
							memcpy(&romBuf[ptr], &addr, 4);
							printf(" -Bigger LZ file detected, repointing to 0x%08X\n", addr);
						}
					}
				}
				fclose(fp);
				fp = NULL;
				addr = 0;
				size = 0;
				ptr = 0;
			}
		}
	}
	printf("Found %d loose sprites in %s!\n", count, folder);
	
	closedir(dir);
}

void GFX_COPY(uint8_t* romBuf, int langID)
{
	// This handles every other sprite
	
	printf("\n");
	
	// First, the Gash Collection item names, there are 200 sprites
	// so to handle it with little code, I suggest adding a header
	// to each file with the position
	
	// NOTE: I'm aware that adding a header isn't very useful here
	// because every sprite is 0x540 in size, but this method guaranteed no
	// extra code if the game decided to change things up.
	
	FILE* fp = NULL;
	int i = 0;
	int itemsPos = 0xBA7CBC;
	const int itemsInc = 0x540;
	for(i = 1;i < 101; ++i) {
		char item[64] = {0};
		sprintf(item, "GFX/GASH_COLLECTION/%s/%03d.bin", LANG_ID[langID], i);
		//printf("GFX: Gash Collection item no.%03d\nFull path: %s", i, item);
		
		fp = fopen(item, "rb");
		if(fp) {
			int header = 0;
			fread(&header, 1, 4, fp);
			if(header == 0x33584647) { //GFX3 backwards
				fread(&header, 1, 4, fp);
				if(header) {
					// fix le
					header = (header & 0xFF) << 24 |
						((header >> 8) & 0xFF) << 16 |
							((header >> 16) & 0xFF) << 8 |
								((header >> 24) & 0xFF);
					//printf("SHOW SIZE: 0x%X\n", header);
					
					fseek(fp, 0x10, SEEK_SET);
					fread(&romBuf[header], 1, itemsInc, fp);
					showMSG(1, i, header);
				}
			}
			else {
				// means no header was found,
				// write file based on position
				fseek(fp, 0, SEEK_SET);
				fread(&romBuf[itemsPos], 1, itemsInc, fp);
				showMSG(1, i, itemsPos);
			}
			fclose(fp);
		}
		else {
			// Read from the universal folder
			sprintf(item, "GFX/GASH_COLLECTION/%03d.bin", i);
			fp = fopen(item, "rb");
			if(fp) {
				fseek(fp, 0, SEEK_SET);
				fread(&romBuf[itemsPos], 1, itemsInc, fp);
				fclose(fp);
				showMSG(1, i, itemsPos);
			}
		}
		itemsPos += itemsInc;
	}
	
	printf("\n");
	
	// GASH COLLECTION unlock requirement hints
	int startPos = 0xB9EABC;
	int sizeReq = 0x600;
	for(i = 1;i < 25; ++i) {
		char item[64] = {0};
		sprintf(item, "GFX/GASH_COLLECTION/%s/req_%02d.bin", LANG_ID[langID], i);
		
		fp = fopen(item, "rb");
		if(fp) {
			fseek(fp, 0, SEEK_SET);
			fread(&romBuf[startPos], 1, sizeReq, fp);
			
			fclose(fp);
			showMSG(2, i, startPos);
		}
		startPos += sizeReq;
	}
	
	// Credits
	fp = NULL;
	int credAddrBase = 0xFD3100; // this is where all repointed credits will stack
	int curSize = 0;
	int baseInc = 0; // If I don't make this align to 32-bits it might break
	
	for(i = 1;i < 29; ++i) {
		char item[64] = {0};
		sprintf(item, "GFX/CREDITS/%s/cred%02d.bin", LANG_ID[langID], i);
		
		fp = fopen(item, "rb");
		if(fp) {
			int header = 0;
			fread(&header, 1, 4, fp);
			if(header == 0x33584647) { //GFX3 backwards
				printf("These files can't have a GFX3 header.\n\n");
			}
			else {
				// means no header was found,
				fseek(fp, 0, SEEK_END);
				curSize = ftell(fp);
				
				if(curSize > arr_cred_size[i-1]) {
					fseek(fp, 0, SEEK_SET);
					fread(&romBuf[credAddrBase+baseInc], 1, curSize, fp);
					fclose(fp);
				
					// update pointer
					int donePtr = (credAddrBase + baseInc) & 0xFF000000;
					if(donePtr == 0x01000000) {
						donePtr = (credAddrBase + baseInc) & 0xFFFFFF;
						donePtr = (credAddrBase + baseInc) | 0x9000000;
					} else {
						donePtr = (credAddrBase + baseInc) & 0xFFFFFF;
						donePtr = (credAddrBase + baseInc) | 0x8000000;
					}
					
					memcpy(&romBuf[arr_cred_ptr[i-1]], &donePtr, 4);
					
					showMSG(4, i, credAddrBase+baseInc);
					baseInc += curSize;
					while(baseInc % 4 != 0)
						++baseInc;
					printf("Repointed credits #%d\n", i);
				}
				else {
					//Read the pos from ptr list
					int jumpAddr = 0;
					memcpy(&jumpAddr, &romBuf[arr_cred_ptr[i-1]], 4);
					jumpAddr &= 0xFFFFFF;
					
					fseek(fp, 0, SEEK_SET);
					fread(&romBuf[jumpAddr], 1, curSize, fp);
					fclose(fp);
					//printf("Wrote credits #%d\n", i);
					printf("\n");
					showMSG(4, i, jumpAddr);
				}
			}
			fclose(fp);
			
			//Gonna have to rely on the tileset existing, but it's not like it's useful otherwise
			sprintf(item, "GFX/CREDITS/%s/cred%02d_map.bin", LANG_ID[langID], i);
			fp = fopen(item, "rb");
			if(fp) {
				fseek(fp, 0, SEEK_END);
				curSize = ftell(fp);
				
				if(curSize > arr_cred_map_size[i-1]) {
					fseek(fp, 0, SEEK_SET);
					fread(&romBuf[credAddrBase+baseInc], 1, curSize, fp);
					fclose(fp);
					
					// update pointer
					int donePtr = (credAddrBase + baseInc) & 0xFF000000;
					if(donePtr == 0x01000000) {
						donePtr = (credAddrBase + baseInc) & 0xFFFFFF;
						donePtr = (credAddrBase + baseInc) | 0x9000000;
					} else {
						donePtr = (credAddrBase + baseInc) & 0xFFFFFF;
						donePtr = (credAddrBase + baseInc) | 0x8000000;
					}
					
					memcpy(&romBuf[arr_cred_ptr[i-1] + 0x10], &donePtr, 4);
					
					showMSG(5, i, credAddrBase+baseInc);
					baseInc += curSize;
					while(baseInc % 4 != 0)
						++baseInc;
					printf("Repointed tilemap for credits #%d\n",i);
				}
				else {
					//no repointing
					//Read the pos from ptr list
					int jumpAddr = 0;
					memcpy(&jumpAddr, &romBuf[arr_cred_ptr[i-1]+0x10], 4);
					jumpAddr &= 0xFFFFFF;
					
					fseek(fp, 0, SEEK_SET);
					fread(&romBuf[jumpAddr], 1, curSize, fp);
					fclose(fp);
					printf("\n");
					showMSG(5, i, jumpAddr);
				}
			}
		}
		else {
			// Read from the universal folder
			sprintf(item, "GFX/CREDITS/cred%02d.bin", i);
			fp = fopen(item, "rb");
			if(fp) {
				fseek(fp, 0, SEEK_END);
				curSize = ftell(fp);
				
				if(curSize > arr_cred_size[i-1]) {
					fseek(fp, 0, SEEK_SET);
					fread(&romBuf[credAddrBase+baseInc], 1, curSize, fp);
					fclose(fp);
				
					// update pointer
					int donePtr = (credAddrBase + baseInc) & 0xFF000000;
					if(donePtr == 0x01000000) {
						donePtr = (credAddrBase + baseInc) & 0xFFFFFF;
						donePtr = (credAddrBase + baseInc) | 0x9000000;
					} else {
						donePtr = (credAddrBase + baseInc) & 0xFFFFFF;
						donePtr = (credAddrBase + baseInc) | 0x8000000;
					}
					
					memcpy(&romBuf[arr_cred_ptr[i-1]], &donePtr, 4);
					
					showMSG(4, i, credAddrBase+baseInc);
					baseInc += curSize;
					while(baseInc % 4 != 0)
						++baseInc;
					printf("Repointed credits #%d\n", i);
				}
				else {
					//Read the pos from ptr list
					int jumpAddr = 0;
					memcpy(&jumpAddr, &romBuf[arr_cred_ptr[i-1]], 4);
					jumpAddr &= 0xFFFFFF;
					
					fseek(fp, 0, SEEK_SET);
					fread(&romBuf[jumpAddr], 1, curSize, fp);
					fclose(fp);
					//printf("Wrote credits #%d\n", i);
					printf("\n");
					showMSG(4, i, jumpAddr);
				}
			}
			// tilemap
			sprintf(item, "GFX/CREDITS/cred%02d_map.bin", i);
			fp = fopen(item, "rb");
			if(fp) {
				fseek(fp, 0, SEEK_END);
				curSize = ftell(fp);
				
				if(curSize > arr_cred_map_size[i-1]) {
					fseek(fp, 0, SEEK_SET);
					fread(&romBuf[credAddrBase+baseInc], 1, curSize, fp);
					fclose(fp);
					
					// update pointer
					int donePtr = (credAddrBase + baseInc) & 0xFF000000;
					if(donePtr == 0x01000000) {
						donePtr = (credAddrBase + baseInc) & 0xFFFFFF;
						donePtr = (credAddrBase + baseInc) | 0x9000000;
					} else {
						donePtr = (credAddrBase + baseInc) & 0xFFFFFF;
						donePtr = (credAddrBase + baseInc) | 0x8000000;
					}
					
					memcpy(&romBuf[arr_cred_ptr[i-1] + 0x10], &donePtr, 4);
					
					showMSG(5, i, credAddrBase+baseInc);
					baseInc += curSize;
					while(baseInc % 4 != 0)
						++baseInc;
					printf("Repointed tilemap for credits #%d\n",i);
				}
				else {
					//no repointing
					//Read the pos from ptr list
					int jumpAddr = 0;
					memcpy(&jumpAddr, &romBuf[arr_cred_ptr[i-1]+0x10], 4);
					jumpAddr &= 0xFFFFFF;
					
					fseek(fp, 0, SEEK_SET);
					fread(&romBuf[jumpAddr], 1, curSize, fp);
					fclose(fp);
					printf("\n");
					showMSG(5, i, jumpAddr);
				}
			}
		}
	}
	
	
	// Generic patch, works by reading all files from x folder and reading the header
	// only supports langID paths, but the cool thing is not worrying about file names
	GenericPatch(romBuf, langID, "MISC");
	GenericPatch(romBuf, langID, "MAIN_MENU");
	GenericPatch(romBuf, langID, "RAIKU_CUP");
	GenericPatch(romBuf, langID, "VS");
	GenericPatch(romBuf, langID, "GASH_COLLECTION");
	GenericPatch(romBuf, langID, "OPTIONS");
}

const char name[] = "Diego A.";
const char noise[] = "MISAKA NETWORK";
const char atk[] = "CHAOS CONTROL";
const char def[] = "IMAGINE BREAKER";
const char quote1[] = "The time-space rift is expanding... There's no more time, I need to hurry.";
const char quote2[] = "Sounds good but you should know only the Power Rangers can control the Mega Voyger.";
const char quote3[] = "Step aside Mercury, it's not you I'm after!";

const char spell1[] = "Zaker";
const char spell2[] = "Rashield";
const char spell3[] = "Jikerdor";
const char spell4[] = "Bao Zakeruga";
const char spell5[] = "Zakeruga";
const char spell6[] = "Rauzaruk";
const char spell7[] = "Zaguruzemu";

const char rando0[] = "Sparkling Wide Pressure";
const char rando1[] = "Exodia the Forbidden One";
const char rando2[] = "Diffusion Wave Motion";

const char artist1[] = "Sara Takatsuki";
const char artist2[] = "Ruka Matsuda";
const char artist3[] = "Suzy Bae";
const char artist4[] = "ChoA";
const char artist5[] = "Haruna Kawaguchi";

const char end1[] = "You foolish man, I am a Jito-Ryo master. Allow me to reunite you with your ancestors.";
const char end2[] = "Jump on panel number one, it will take you to panels two and three.";

int main (int argc, char *argv[])
{
  int i = 0;
  int x = 0;
  int fullLimit = 0;
  int halfLimit = 0;
  int halfLimit1 = 0; //for recording game string len
  int fullLimit1 = 0;
  bool lineBroke = false;
  int checksum = 0;

  //verify data
  for (i = 0; i < 8; ++i)
     checksum += name[i];
  for (i = 0; i < 13; ++i)
     checksum += noise[i];
  for (i = 0; i < 10; ++i)
     checksum += spell7[i];
  //printf("SUM: 0x%X\n\n", checksum);
#if 1
  if(checksum != 0x0A6B) {
     printf("\n\n");
     printf("An error has been detected.\n");
     printf("You do not have enough power-from-within!\n");
     return 0;
  }
#endif

  char outName[128] = {0};
  int langID = 1; // English
  bool langChanged = false;
  bool isROMpatched = true;
  bool verbose = false;
  
  // specific patches
  bool orb_cheat = false;
  bool riya_vo_fix = false;
  bool noTitle_vo = false;
  bool faudo_cheat = false;
  uint8_t forceVoice = 0;
  
  // Track 028 is a bit longer when replaced
  // romBuf[0xFA34E9] = 0xB0 (original 81)
  
  // GBA's sound engine overrides
  uint8_t engineRate = 0;     // Forces a supported sample rate
  uint8_t engineChannels = 0; // Reduces the game's sample channels
  
  // For enabling ipatix's hq mixer
  bool hq_mixer = false;
  
  //remove initial Fight! vo
  //romBuf[0x01096E] = 0; //u32 it's a bl after all
  
  // Character Intro vo bl
  //romBuf[0x83AC0] = 0;
  //romBuf[0x83AC1] = 0;
  
  //Always start with max orb
  //0800594C = 2721

  // Zagurzem timer
  // 0x02003DB0 (short) = original value is 0x1A4
  // value found on rom at 0xFAC9A4
  uint16_t zagurzemTimer = 0;
  
  // flag to avoid applying easter egg patch
  bool skipDTBadge = false;
  
  // prints the address to a specific voice
  int specificVoice = -1;
  
  // mode for replacing voices
  bool vo_replace = false;
  
  // mode for replacing music
  bool music_replace = false;
  
  // only affects stage BGM
  bool mute_music = false;
  
  // To mute all sound in the game
  // 0807C436 = blank this bl
  
  // sample rate in Hz of all replaced sounds
  uint32_t srate = 11025;
  
  uint32_t DTT_ROMSize = 16*1024*1024;
  bool expandROM = false;
  
  // This is the amount of free space at the bottom of the original ROM 0xFC1590
  uint32_t ogFreeSpace = 0x03EA70;

  printf("\n\n");
  printf("~ Konjiki no Gash Bell!! Dream Tag Tournament: Script Converter ~  by %s\n\n\n", name);
  
  //printf("Arg count: %d \n\n", argc);

  if(argc > 1) {
    for(i = 1; i < argc;) {
		
		if(strcmp(argv[i], "-o") == 0) // name of the output, this is optional
        {
                ++i;   //skip arg
                x = 0; //reset parse
				//printf("\n");
				
				sprintf(outName, "%s", argv[i]);
				
				++i;
				
                printf("Set output filename to: %s\n\n", outName);
        }
		
		if(strcmp(argv[i], "-l") == 0) // language to encode to from the text file
        {
                ++i;   //skip arg
                x = 0; //reset parse
				//printf("\n");
				
				if(strcmp("English", argv[i]) == 0)
					langID = 1;
				else if(strcmp("Spanish", argv[i]) == 0)
					langID = 2;
				else if(strcmp("French", argv[i]) == 0)
					langID = 3;
				else if(strcmp("Italian", argv[i]) == 0)
					langID = 4;
				else if(strcmp("Portuguese", argv[i]) == 0)
					langID = 5;
				else if(strcmp("German", argv[i]) == 0)
					langID = 6;
				else if(strcmp("Japanese", argv[i]) == 0)
					langID = 0;
				else
					langID = 100;
				
				langChanged = true;
				
                //printf("Language set to: %s\n\n", LANG_GET[langID]);
				if(langID < 100)
					printf("Language set to: %s\n\n", argv[i]);
				else {
					printf("Language not properly set, using English...\n\n");
					langID = 1;
				}
				++i;
				continue;
        }
		
		if(strcmp(argv[i], "--not-patched") == 0) // if the ROM is not patched some characters represent JP characters
        {
                ++i;   //skip arg
                x = 0; //reset parse
				//printf("\n");
				
				// WHAT, I don't think this is needed anymore...
				
				isROMpatched = false;
				printf("Decoding set to original Japanese characters.\n\n");
				continue;
        }
		
		if(strcmp(argv[i], "--vo-id") == 0)
        {
                ++i;   //skip arg
                x = 0; //reset parse
				
				specificVoice = atoi(argv[i]);
				
				if(specificVoice > 213) {
					printf("The selected ID is invalid, use 0 to 213 range.\n");
					specificVoice = 0;
				}
				
				++i;
				printf("VOICE finder set...\n\n");
				continue;
        }
		
		if(strcmp(argv[i], "--vo-samplerate") == 0)
        {
                ++i;   //skip arg
                x = 0; //reset parse
				
				if(atoi(argv[i]) < 48050) {
					srate = atoi(argv[i]);
					printf("VO sample rate will be changed... %dHz\n\n", srate);
				}
				
				++i;
				continue;
        }
		
		if(strcmp(argv[i], "--vo-replace") == 0)
        {
                ++i;   //skip arg
                x = 0; //reset parse
				
				vo_replace = true;
				printf("VOICE Replacement set...\n\n");
				continue;
        }
		
		if(strcmp(argv[i], "--music-replace") == 0)
        {
                ++i;   //skip arg
                x = 0; //reset parse
				
				music_replace = true;
				printf("MUSIC Replacement set...\n\n");
				continue;
        }
		
		if(strcmp(argv[i], "--mute-bgm") == 0)
        {
                ++i;   //skip arg
                x = 0; //reset parse
				
				mute_music = true;
				printf("All stage BGM will be muted...\n\n");
				continue;
        }
		
		if(strcmp(argv[i], "--zagurzem") == 0 || strcmp(argv[i], "--zaguruzemu") == 0)
        {
                ++i;   //skip arg
                x = 0; //reset parse
				
				if(atoi(argv[i]) > 0 && atoi(argv[i]) < 121) {
					zagurzemTimer = atoi(argv[i]);
					printf("Zagurzem cooldown set to... %d seconds\n\n", zagurzemTimer);
				}
				
				++i;
				continue;
        }
		
		if(strcmp(argv[i], "--op") == 0)
        {
                ++i;   //skip arg
                x = 0; //reset parse
				
				orb_cheat = true;
				printf("You have gained access to the golden spell book...\n\n");
				continue;
        }
		
		if(strcmp(argv[i], "--faudo-cheat") == 0)
        {
                ++i;   //skip arg
                x = 0; //reset parse
				
				faudo_cheat = true;
				printf("Faudo minigame patch set...\n\n");
				continue;
        }
		
		if(strcmp(argv[i], "--skip-badge") == 0)
        {
                ++i;   //skip arg
                x = 0; //reset parse
				
				skipDTBadge = true;
				printf("DT Badge easter egg will not be applied...\n\n");
				continue;
        }
		
		if(strcmp(argv[i], "--force-intro-vo") == 0)
        {
                ++i;   //skip arg
                x = 0; //reset parse
				
				if(atoi(argv[i]) > 0 && atoi(argv[i]) < 3) {
					forceVoice = atoi(argv[i]);
					printf("Character intro voice forced to... P%d\n\n", forceVoice);
				}
				continue;
        }
		
		if(strcmp(argv[i], "--riya-fix") == 0)
        {
                ++i;   //skip arg
                x = 0; //reset parse
				
				riya_vo_fix = true;
				printf("Riya VO fix set...\n\n");
				continue;
        }
		
		if(strcmp(argv[i], "--no-title-vo") == 0)
        {
                ++i;   //skip arg
                x = 0; //reset parse
				
				noTitle_vo = true;
				printf("Removing title screen VO... (Yujo no Zakeru!/Dream Tag Tournament!)\n\n");
				continue;
        }
		
		if(strcmp(argv[i], "--force-samplerate") == 0)
        {
                ++i;   //skip arg
                x = 0; //reset parse
				
				if(atoi(argv[i]) < 13) {
					engineRate = atoi(argv[i]);
					printf("Game's sample rate will be changed...\n\n");
				}
				
				++i;
				continue;
        }
		
		if(strcmp(argv[i], "--force-channels") == 0)
        {
                ++i;   //skip arg
                x = 0; //reset parse
				
				if(atoi(argv[i]) < 13) {
					engineChannels = atoi(argv[i]);
					printf("Game's channel limit will be reduced...\n\n");
				}
				
				++i;
				continue;
        }
		
		if(strcmp(argv[i], "--hq-mixer") == 0)
        {
                ++i;   //skip arg
                x = 0; //reset parse
				
				hq_mixer = true;
				printf("Using ipatix's high quality sound mixer...\n\n");
				continue;
        }
		
		if(strcmp(argv[i], "--more-space") == 0)
        {
                ++i;   //skip arg
                x = 0; //reset parse
				
				expandROM = true;
				DTT_ROMSize = 32*1024*1024;
				printf("ROM expansion to 32MB set...\n\n");
				continue;
        }
		
		if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "/help") == 0)
        {
			++i;   //skip arg
            x = 0; //reset parse
			
			printf("How to use:\n\ndtt_conv.exe -l Spanish -e script_dec.txt\n\nThis will convert ");
			printf("all the decoded script into the DTT format, and output the ROM with it patched in.\n");
			printf("The language code is English by default, so it doesn't need to be included.\n\n");
			
			printf("You can use: dtt_conv -d dtt.gba\nto decode the script from a clean ROM.\n\n");
			
			printf("You can also use: dtt_conv -x dtt.gba\nThis will extract all the voice files ");
			printf("to the VO/JP directory.\nIf you pass --vo-id along with the ID, it will print ");
			printf("the ROM address where the VO entry starts.\n");
			
			printf("If you pass --vo-replace it will try to find VO files in VO/EN to replace.\n");
			printf("Using the -l arg to change language also works.\n\n");
			
			printf("If you pass --music-replace it will try to find streams in VO/MUSIC to replace.\n");
			printf("Streamed music is encoded just like VO samples.\n");
			printf("This means that only monaural music will work.\n");
			printf("Currently, the rate must be 11025 Hz unless you specify.\n");
			printf("Loop is supported, you must open your PCM sample in a hex editor\n");
			printf("and insert (not write) 8 bytes to the start position,\n");
			printf("on the first 4 bytes write the word -LOOP- this is an ID to detect it,\n");
			printf("the second 4 bytes contain the sample number where the loop occurs.\n");
			printf("See the examples.\n\n");
			
			printf("Use --more-space to expand ROM to 32MB and allow inserting bigger sounds.\n\n");
			
			printf("Use --no-title-vo to remove the VO calls on the title screen,\n");
			printf("except for the -Konjiki no Gash!- call.\n");
			printf("They will still appear in the Sound Test as 001 and 002.\n\n");
			
			printf("--force-intro-vo X will add code that makes the character intro voices,\n");
			printf("the ones that play at the start of a match, only play for X player.\n");
			printf("Example: --force-intro-vo 2 makes player 2's intro VO play but not player 1.\n\n");
			
			printf("Try --vo-samplerate 10512 to keep the original game pitch on replaced sounds.\n");
			printf("By default it's 11025, adjust depending on your sounds.\n\n");
			
			printf("--mute-bgm will silence all stage BGM.\n\n");
			
			printf("--zagurzem X allows setting the number of seconds the spell will last.\n");
			printf("The original is 7 seconds. There is a limit of 120 seconds.\n\n");
			printf("--riya-fix prevents Riya's intro VO from getting cut.\n\n");
			printf("--faudo-cheat to force the Faudo Race minigame to always be easy.\n\n");
			printf("--force-samplerate to adjust quality of samples.\n");
			printf("0=original (13379 Hz) 1=5734 Hz, 2=7884 Hz, 3=10512 Hz, 4=13379 Hz,\n");
			printf("5=15768 Hz, 6=18157 Hz, 7=21024 Hz, 8=26758 Hz, 9=31536 Hz, 10=36314 Hz,\n");
			printf("11=40137 Hz, 12=42048 Hz\n");
			printf("Going over 7 makes the game too slow, unless you use the hq mixer.\n\n");
			
			printf("--hq-mixer will replace the sound mixer with ipatix's version.\n");
			printf("It improves performance, audio quality, and removes audio pops.\n\n");
			
			printf("--force-channels set the limit of channels the game uses to play samples.\n");
			printf("The game uses all 12 channels by default, lowering it may help reduce pops.\n\n");
			
			printf("--op to quickly gain max SP.\n\n");
			
			//Show dtt.sav current checksum and calculate manually
			FILE * fdtt = fopen("out_dtt.sav", "rb");
			if(fdtt) {
				dtt_save = malloc(sizeof(SaveData));
				if(!dtt_save) {
				//	printf("Couldn't locate out_dtt.sav!\n\n");
					fclose(fdtt);
					break;
				}
				
				// just look at the first slot
				fseek(fdtt, 0, SEEK_SET);
				fread(dtt_save, 1, 0x48, fdtt);
				fclose(fdtt);
				
				uint16_t res_checksum = 0;
				int d = 0;
				for(d=0;d<0x20;d++)
					res_checksum += dtt_save->data[d];
				
				printf("Note: EEPROM saves are often swapped, breaking calculation.\n");
				printf("Found save!\nCurrent checksum: 0x%X, Calculated: 0x%X\n\n",
						dtt_save->checksum2, res_checksum);
				
				free(dtt_save);
			}
			
			break;
		}
		
		if(strcmp(argv[i], "--tilemap-fixer") == 0) // Modify tilemaps to use a different palette
        {
			++i;   //skip arg
            x = 0; //reset parse
			
			tilemapIndexer(0x4B0, "tilemap.bin");
			
			printf("Finished tilemap fixes!\n\n");
			
			break;
		}
		
		if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) // Print more info on console
        {
                ++i;   //skip arg
                x = 0; //reset parse
				//printf("\n");
				
				verbose = true;
				printf("Verbosity set.\n\n");
				continue;
        }
		
		if(strcmp(argv[i], "-e") == 0) // open a script text file and encode it directly to the ROM
        {
                ++i;   //skip arg
                x = 0; //reset parse
                //printf("\n");
                printf("Script Encoding Mode\n\n");
				
				int fileSz = 0;
				
				FILE *fp = fopen(argv[i], "rb");
				if(fp == NULL) {
					printf("Failed to open file!\n\nAre you reading the wrong spell book?\n\n");
					break;
				}
				
				// get file size
				fseek(fp, 0, SEEK_END);
				fileSz = ftell(fp);
				
				fseek(fp, 0, SEEK_SET);
				
				bool linesFound = false;
				unsigned lines = 0;
				char searchID[8] = {0};
				char txtRead[8] = {0};
				sprintf(searchID, "%s: ", LANG_ID[langID]);
				
				int p = 0;
				for(p = 0; p < fileSz; ++p) {
					fseek(fp, p, SEEK_SET);
					fread(txtRead, 1, 4, fp);
					
					if(!linesFound && strncmp(txtRead, "ln: ", 4) == 0) {
						fread(txtRead, 1, 4, fp);
						lines = atoi(txtRead);
						linesFound = true;
						
						// This is just to prevent incorrect scripts from going overboard
						if(lines > 700)
							lines = 617;
						
						printf("Lines specified by script: %d\n", lines);
					}
				}
				
				if(lines == 0) {
					printf("No lines to encode!\n");
					fclose(fp);
					break;
				}
				
				// Build the base binary
				LinesData * ptrTable = (LinesData *) malloc((lines*4)+4);
				
				// Should always be 0 but doesn't have to be
				ptrTable[0].lineptr = 0;
				
				// Allocate for encoded string
				uint8_t dstBuf[96*1024] = {0};
				
				//show me
				//printf("Show size: 0x%X\n", fileSz);
				
				// Quote hack so I don't have to modify the script
				unsigned initialQuote = 1;
				
				// Position of dst, needed to deal with the mixed int lengths
				int s = 0;
				
				bool dontNul = false;
				char tmpLine[1024] = {0};
				int lineCNT = 0;
				int curPTR = 0;
				int a = 0;
				for(p = 0; p < fileSz; ++p) {
					fseek(fp, p, SEEK_SET);
					fread(txtRead, 1, 4, fp);
					if(linesFound && strncmp(txtRead, searchID, 4) == 0) {
						++lineCNT;
						
						if(!verbose)
							printf("\rFound string! %d\x1b[K", lineCNT);
						
						// Reset after every line read
						curPTR = 0;
						a = 0;
						
						while(1) {
							int linebrk = 0;
							++a;
							fseek(fp, p+a, SEEK_SET);
							fread(&linebrk, 1, 1, fp);
							
							// This keeps track of the string size
							++curPTR;
							
							if(linebrk == 0x0D || linebrk == 0x0A || linebrk == 0 || curPTR > 1024)
								break;
						}
						fseek(fp, p+4, SEEK_SET);
						fread(tmpLine, 1, curPTR-4, fp);
						
						if(verbose)
							printf("String: %s, Size: %d, Line: %d\n", tmpLine, curPTR, lineCNT);
						
						int e = 0;
						for(e = 0; e < curPTR-4; ++e, s+=2) {
							// Check if there's a command
							if(tmpLine[e] == '\\') {
								if(verbose)
									printf("Using special command.\n");
								
								uint8_t val_1 = 0;
								uint8_t val_2 = 0;
								uint8_t val_3 = 0;
								
								if(strncmp(&tmpLine[e+1], "RESTORE", 7) == 0) {
									val_1 = 0x25;
									val_2 = 0x49;
									//val_3 = 0x31;
									dstBuf[s] = val_1;
									dstBuf[s+1] = val_2;
								//	dstBuf[s+2] = val_3;
									
									e += 7; // characters to read
									//s += 1; // characters to skip
									printf("Incorrect command, this isn't Makai no Bookmark!\n");
								}
								else if(strncmp(&tmpLine[e+1], "N", 1) == 0) {
									val_1 = 0x42;
									val_2 = 0x52;
									dstBuf[s]   = val_1;
									dstBuf[s+1] = val_2;
									
									e += 1;
								}
								else if(strncmp(&tmpLine[e+1], "BR", 2) == 0) {
									val_1 = 0x42;
									val_2 = 0x52;
									dstBuf[s]   = val_1;
									dstBuf[s+1] = val_2;
									
									e += 2;
								}
								else if(strncmp(&tmpLine[e+1], "LEFT", 4) == 0) {
									val_1 = 0x4C;
									val_2 = 0x46;
									dstBuf[s]   = val_1;
									dstBuf[s+1] = val_2;
									
									e += 4;
								}
								else if(strncmp(&tmpLine[e+1], "LF", 2) == 0) {
									val_1 = 0x4C;
									val_2 = 0x46;
									dstBuf[s]   = val_1;
									dstBuf[s+1] = val_2;
									
									e += 2;
								}
								else if(strncmp(&tmpLine[e+1], "END", 3) == 0) {
									val_1 = 0x45;
									val_2 = 0x44;
									dstBuf[s]   = val_1;
									dstBuf[s+1] = val_2;
									
									e += 3;
								}
								else if(strncmp(&tmpLine[e+1], "ED", 2) == 0) {
									val_1 = 0x45;
									val_2 = 0x44;
									dstBuf[s]   = val_1;
									dstBuf[s+1] = val_2;
									
									e += 2;
								}
								else if(strncmp(&tmpLine[e+1], "ARROW", 5) == 0) {
									val_1 = 0x81;
									val_2 = 0xA8; // ->
									
									if(strncmp(&tmpLine[e+6], "RIGHT", 5) == 0) {
										val_2 = 0xA8;
										e += 5;
									} else if(strncmp(&tmpLine[e+6], "LEFT", 4) == 0) {
										val_2 = 0xA9;
										e += 4;
									} else if(strncmp(&tmpLine[e+6], "UP", 2) == 0) {
										val_2 = 0xAA;
										e += 2;
									} else if(strncmp(&tmpLine[e+6], "DOWN", 4) == 0) {
										val_2 = 0xAB;
										e += 4;
									} else {
										printf("Arrow command randomized.\n");
									}
									
									dstBuf[s]   = val_1;
									dstBuf[s+1] = val_2;
									
									e += 5;
								}
								else if(strncmp(&tmpLine[e+1], "MULT", 4) == 0) {
									val_1 = 0x81;
									val_2 = 0x7E;
									dstBuf[s]   = val_1;
									dstBuf[s+1] = val_2;
									
									e += 4;
								}
								else if(strncmp(&tmpLine[e+1], "DEG", 3) == 0) {
									val_1 = 0x81;
									val_2 = 0x42;
									dstBuf[s]   = val_1;
									dstBuf[s+1] = val_2;
									
									e += 3;
								}
								else if(strncmp(&tmpLine[e+1], "HEART", 5) == 0) {
									val_1 = 0x87;
									val_2 = 0x40;
									dstBuf[s]   = val_1;
									dstBuf[s+1] = val_2;
									
									e += 5;
								}
								else if(strncmp(&tmpLine[e+1], "LOVE", 4) == 0) {
									val_1 = 0x87;
									val_2 = 0x40;
									dstBuf[s]   = val_1;
									dstBuf[s+1] = val_2;
									
									e += 4;
								}
								else if(strncmp(&tmpLine[e+1], "MUSIC", 5) == 0) {
									val_1 = 0x81;
									val_2 = 0xF4;
									dstBuf[s]   = val_1;
									dstBuf[s+1] = val_2;
									
									e += 5;
								}
								
								// TODO: any other symbols?
								
								
							} else if(tmpLine[e] != 0) {
								// Security
						//		if(ConvertChar(tmpLine[e]) == 0)
						//			break;
								
								uint8_t val = 0;
								
								// Handle Fullwidth encoding
								if((tmpLine[e] & 0xF0) == 0xC0) {
									// Symbol or tilde - 2 bytes
									if(ConvertCharSymb(tmpLine[e], tmpLine[e+1]) == 0)
										break;
									
									val = ConvertCharSymb(tmpLine[e], tmpLine[e+1]) >> 8;
									dstBuf[s] = val;
								}
							#if 0
								else if((tmpLine[e] & 0xF0) == 0xE0) {
									// JP character - 3 bytes
									if(ConvertCharJP(tmpLine[e], tmpLine[e+1], tmpLine[e+2]) == 0)
										break;
									
									val = ConvertCharJP(tmpLine[e], tmpLine[e+1], tmpLine[e+2]) >> 8;
									dstBuf[s] = val;
								}
							#endif
								else if(((tmpLine[e] & 0xF0) == 0x80 && (tmpLine[e] & 0xFF) != 0x85) || (tmpLine[e] & 0xF0) == 0x90) {
									// Keep as is
									val = tmpLine[e];
									dstBuf[s] = val;
								} else {
									if(ConvertChar(tmpLine[e]) == 0)
										break;
									
									val = ConvertChar(tmpLine[e]) >> 8;
									
									// PART 1
									// Fix accented i problem with it being a 3 width character
									// The character exists twice in the data, one is shifted right by 1px
									// 0x8394 = katakana value that's now i with tilde.
									// 0x82BC = regular i with tilde.
									if(val == 0x82 && (ConvertChar(tmpLine[e]) & 0xFF) == 0xBC) {
										// Check if the following character is a->Z
										if(tmpLine[e+1] > 0x40 && tmpLine[e+1] < 0x7A) {
											val = 0x83;
										//	printf("Corrected i tilde: %s\n", &tmpLine[e]);
										}
									}
									
									dstBuf[s] = val;
								}
								
								// Diffusion Wave Motion
								if((tmpLine[e] & 0xF0) == 0xC0) {
									val = ConvertCharSymb(tmpLine[e], tmpLine[e+1]);
									dstBuf[s+1] = val;
									
									// end by skipping 1 byte
									++e;
								}
							#if 0
								else if((tmpLine[e] & 0xF0) == 0xE0) {
									val = ConvertCharJP(tmpLine[e], tmpLine[e+1], tmpLine[e+2]);
									dstBuf[s+1] = val;
									
									// end by skipping 2 bytes
									e += 2;
								}
							#endif
								else if(((tmpLine[e] & 0xF0) == 0x80 && (tmpLine[e] & 0xFF) != 0x85) || (tmpLine[e] & 0xF0) == 0x90) {
									// Keep as is
									val = tmpLine[e+1];
									dstBuf[s+1] = val;
									++e;
								} else {
									val = ConvertChar(tmpLine[e]);
									
									// quotes hack
									if(val == 0x68 && (ConvertChar(tmpLine[e]) >> 8) == 0x81) {
										if(initialQuote)
											val = 0x67;
										initialQuote ^= 1;
										
									//	if(initialQuote)
									//		printf("InitialQuote: %s\n\n", &tmpLine[e]);
									}
									
									// PART 2
									// Fix accented i problem with it being a 3 width character
									// The character exists twice in the data, one is shifted right by 1px
									// 0x8394 = katakana value that's now i with tilde.
									// 0x82BC = regular i with tilde.
									if(val == 0xBC && (ConvertChar(tmpLine[e]) >> 8) == 0x82) {
										// Check if the following character is a->Z
										if(tmpLine[e+1] > 0x40 && tmpLine[e+1] < 0x7A) {
											val = 0x94;
										//	printf("Corrected_2 i tilde: %s\n", &tmpLine[e]);
										}
									}
									
									dstBuf[s+1] = val;
								}
							} else {
								// No choice but to break
								dontNul = true;
								break;
							}
						}
						
						// When the line is done write null bytes until it's 32-bit aligned
						if(!dontNul) {
							uint8_t adv = 2;
							
							while((s+adv) % 4 != 0) {
								dstBuf[s+adv] = 0;
								++adv;
							}
							s += adv;
						}
						dontNul = false;
						
						// Perfect time to add the pointer
						if((lineCNT+1) < (lines+1))
							ptrTable[lineCNT].lineptr = s;
						
						// Clear the buffer for the next line
						memset(tmpLine, 0, sizeof(tmpLine));
						
						// Let the ln tag control how many lines to process
						if(lineCNT == lines)
						//if(lineCNT == 2)
							break;
					}
				}
				
				printf("\n\n");
				
				// show me
				//printf("SHOW: 0x%X\n", ptrTable[0].lineptr);
				
				// Adjust the pointers, 0xFB0430
				for(p = 0; p < lineCNT; ++p)
					ptrTable[p].lineptr += 0x08FC1590;
				
				// Load ROM to buffer
				uint8_t* romBuf = malloc(DTT_ROMSize);
				FILE *fb = fopen("dtt.gba", "rb");
				if(fb == NULL) {
					printf("Failed to open ROM.\nMake sure you have 'DTT.gba' on the same dir.\n");
					break;
				}
				
				// get file size
				fseek(fb, 0, SEEK_SET);
				fseek(fb, 0, SEEK_END);
				fileSz = ftell(fb);
				
				if(fileSz < 16*1024*1024) {
					printf("You are mistaken! This isn't the real ROM!\n");
					fclose(fp);
					break;
				}
				
				fseek(fb, 0, SEEK_SET);
				fread(&fileSz, 1, 4, fb);
				
				if(fileSz != 0xEA00002E)
					printf("This doesn't look like a GBA ROM!\n");
				
				// final pointer address 0xFB0430
				// start pointer address 0xFAFA94
				
				fseek(fb, 0, SEEK_SET);
				fread(romBuf, 1, expandROM ? DTT_ROMSize/2 : DTT_ROMSize, fb);
				fclose(fb);
				
				// Replace the pointers
				memcpy(&romBuf[0xFAFA94], ptrTable, lineCNT*4);
				
				// Write the new strings at the end
				memcpy(&romBuf[0xFC1590], dstBuf, s);
				
				// Try to load Minish font
				fb = fopen("GFX/08223AF4_DTT_KANA.bin", "rb");
				if(fb) {
					fseek(fb, 0, SEEK_SET);
					fseek(fb, 0, SEEK_END);
					fileSz = ftell(fb);
					
					if(fileSz <= 0x12BC) {
						fseek(fb, 0, SEEK_SET);
						fread(&romBuf[0x223AF4], 1, fileSz, fb);
						printf("Applied font.\n");
					} else {
						printf("Font file too big.\n");
						// pointer at 3 locations
						uint32_t reAddr = 0xFC1590 + s + 0x10;
						romBuf[0x6F3E0] = reAddr;
						romBuf[0x6F3E1] = reAddr >> 8;
						romBuf[0x6F3E2] = reAddr >> 16;
						
						romBuf[0x6F4F0] = reAddr;
						romBuf[0x6F4F1] = reAddr >> 8;
						romBuf[0x6F4F2] = reAddr >> 16;
						
						romBuf[0x6F79C] = reAddr;
						romBuf[0x6F79D] = reAddr >> 8;
						romBuf[0x6F79E] = reAddr >> 16;
						
						fseek(fb, 0, SEEK_SET);
						fread(&romBuf[reAddr], 1, fileSz, fb);
						
						printf("Applied and repointed font.\n");
					}
					
					// VWF parameters
						romBuf[0xFB6BFC] = 0x4; // FULLWIDTH SPACE, OG: 0xC
						
						romBuf[0xFB6C06] = 0x8; // OG: 0xA, asterisk
						romBuf[0xFB6C0A] = 0xA; // @
						romBuf[0xFB6C0B] = 2; // apostrophe
						romBuf[0xFB6C0C] = 6; // 0
						romBuf[0xFB6C0D] = 6;
						romBuf[0xFB6C0E] = 6;
						romBuf[0xFB6C0F] = 6;
						romBuf[0xFB6C10] = 6;
						romBuf[0xFB6C11] = 6;
						romBuf[0xFB6C12] = 6;
						romBuf[0xFB6C13] = 6;
						romBuf[0xFB6C14] = 8; // number 8 is 7 but only even values work
						romBuf[0xFB6C15] = 6;
						romBuf[0xFB6C16] = 6; // A
						romBuf[0xFB6C17] = 6;
						romBuf[0xFB6C18] = 8; // C is 7 but
						romBuf[0xFB6C19] = 6;
						romBuf[0xFB6C1A] = 6;
						romBuf[0xFB6C1B] = 6;
						romBuf[0xFB6C1C] = 8; // G is 7
						romBuf[0xFB6C1D] = 8; // H is 7
						romBuf[0xFB6C1E] = 4; // I is 5 but 4 works OK
						romBuf[0xFB6C1F] = 6;
						romBuf[0xFB6C20] = 8; // K is 7
						romBuf[0xFB6C21] = 8; // L is 7, 6 looks good
						romBuf[0xFB6C22] = 6;
						romBuf[0xFB6C23] = 6;
						romBuf[0xFB6C24] = 8; // O is 7
						romBuf[0xFB6C25] = 6;
						romBuf[0xFB6C26] = 8; // Q is 7
						romBuf[0xFB6C27] = 6;
						romBuf[0xFB6C28] = 6;
						romBuf[0xFB6C29] = 6;
						romBuf[0xFB6C2A] = 8; // U is 7
						romBuf[0xFB6C2B] = 6; // V is 7
						romBuf[0xFB6C2C] = 6; // W is 5
						romBuf[0xFB6C2D] = 8; // X is 7
						romBuf[0xFB6C2E] = 6; // Y is 5
						romBuf[0xFB6C2F] = 6;
						
						// Kana replacements for lowercase
						romBuf[0xFB6C30] = 6; // a, should be 7
						romBuf[0xFB6C31] = 6; // b
						romBuf[0xFB6C32] = 6; // c
						romBuf[0xFB6C33] = 6;
						romBuf[0xFB6C34] = 6;
						romBuf[0xFB6C35] = 4; // f
						romBuf[0xFB6C36] = 6; // g
						romBuf[0xFB6C37] = 6;
						romBuf[0xFB6C38] = 2;
						romBuf[0xFB6C39] = 4;
						romBuf[0xFB6C3A] = 6;
						romBuf[0xFB6C3B] = 2;
						romBuf[0xFB6C3C] = 6;
						romBuf[0xFB6C3D] = 6;
						romBuf[0xFB6C3E] = 6;
						romBuf[0xFB6C3F] = 6;
						romBuf[0xFB6C40] = 6;
						romBuf[0xFB6C41] = 4; // r, actually 5
						romBuf[0xFB6C42] = 6;
						romBuf[0xFB6C43] = 4; // t, actually 5
						romBuf[0xFB6C44] = 6;
						romBuf[0xFB6C45] = 6;
						romBuf[0xFB6C46] = 6;
						romBuf[0xFB6C47] = 6;
						romBuf[0xFB6C48] = 6;
						romBuf[0xFB6C49] = 6; // z
						romBuf[0xFB6C4A] = 6; // ñ
						romBuf[0xFB6C4B] = 6; // a tilde
						romBuf[0xFB6C4C] = 6; // e tilde
						romBuf[0xFB6C4D] = 4; // i tilde
						romBuf[0xFB6CD0] = 4; // alternate i tilde that's centered
						romBuf[0xFB6C4E] = 6; // o tilde
						romBuf[0xFB6C4F] = 6; // u tilde
						romBuf[0xFB6C50] = 6; // u dia
						romBuf[0xFB6C51] = 6; // cedilla
						romBuf[0xFB6C52] = 6; // inverted ?
						romBuf[0xFB6C53] = 2; // inverted !
						
						// Symbols
						romBuf[0xFB6CD2] = 4; // , OG: 6
						romBuf[0xFB6CD8] = 4; // . OG: 8
						romBuf[0xFB6CDC] = 6; // ( OG: 0xC
						romBuf[0xFB6CDD] = 6; // ) OG: 0xC
					//	romBuf[0xFB6CE4] = 6; // " initial, OG: 0x08, requires fixing the entire script...
						romBuf[0xFB6CE5] = 6; // " OG: 0x08, should be safe to enable
						romBuf[0xFB6CF1] = 6; // ? OG: 0xC
						romBuf[0xFB6CF2] = 6; // ! OG: 0xC
						romBuf[0xFB6CF3] = 0xA; // ... OG: 0xE
						romBuf[0xFB6CF5] = 0xA; // - OG: 0xC
						
						if(faudo_cheat) {
							// Faudo difficulty decider -- points difficulty to EASY for all
							romBuf[0x078DA8] = 0x14; // 07CD14 14CD0708; //og= 14 1F 02 02
							romBuf[0x078DA9] = 0xCD;
							romBuf[0x078DAA] = 0x07;
							romBuf[0x078DAB] = 0x08;
						}
						
						// Fix x pos for POINTS sprite that appears in results screen minigames
						romBuf[0x237032] = 4;    // xpos of first 32 pixels
						romBuf[0x237038] = 0x24; // xpos of last 16 pixels
						
						// 1px fix for the equal signs in minigame results
						romBuf[0x237014] = 1;
						
						// Pad this area out
						memcpy(&romBuf[0xFFFE50], msgEOL, 0x20);
						
						// Faudo minigame cheat alt -- increase overall score calculation (doesn't help if you don't win)
						//romBuf[0xFB7AB4] = 0x96; //og 0x64
						
						// Faudo minigame slow faudo -- if you A+B too much the score will ruin the save and softlock
						//romBuf[0x078CC2] = 0;
						//romBuf[0x078CC3] = 0;
						
						// ORB code at 0x012192 0xE bytes can be nulled to always show 3 orbs.
						// This won't actually give you 3 orbs, though.
						// 0x020220D0 = 0x1E = select btn MAX, actual max is 0x27
						if(orb_cheat) {
							// Any increase or decrease of SP will maximize it
							romBuf[0xEE40] = 0; // OG hex: 9042
							romBuf[0xEE41] = 0; // OG arm: cmp r0,r2
							// Any attack or damage will trigger max tag action orbs
							romBuf[0xEE7E] = 0; // OG hex: 01DD
							romBuf[0xEE7F] = 0; // OG arm: ble
						}
						
						// Zagurzem lasts 7 seconds
						if(zagurzemTimer) {
							zagurzemTimer *= 60;
							uint8_t val_1 = zagurzemTimer;
							uint8_t val_2 = zagurzemTimer >> 8;
							romBuf[0xFAC9A4] = val_1;
							romBuf[0xFAC9A5] = val_2;
						}
						
						if(!skipDTBadge) {
							// Easter Egg method of unlocking the Dream Tag Badge
							memcpy(&romBuf[0xFFFAF0], dtt_badge_bin, sizeof(dtt_badge_bin));
							romBuf[0x15718] = 0;
							romBuf[0x15719] = 0x49;
							romBuf[0x1571A] = 0x8F;
							romBuf[0x1571B] = 0x46;
							romBuf[0x1571C] = 0xF0;
							romBuf[0x1571D] = 0xFA;
							romBuf[0x1571E] = 0xFF;
							romBuf[0x1571F] = 0x08;
						}
						
						if(forceVoice) {
							// 004B 9F46 80FE FF08
							
							// branch to the new code
							romBuf[0x83AB4] = 0;
							romBuf[0x83AB5] = 0x4B;
							romBuf[0x83AB6] = 0x9F;
							romBuf[0x83AB7] = 0x46;
							romBuf[0x83AB8] = 0x80;
							romBuf[0x83AB9] = 0xFE;
							romBuf[0x83ABA] = 0xFF;
							romBuf[0x83ABB] = 0x08;
							
							// added code to the bottom of the 16 MB ROM
							memcpy(&romBuf[0xFFFE80], forceVoiceIntro_bin, sizeof(forceVoiceIntro_bin));
							
							// Determine wanted player
							if(forceVoice == 1) {
								// Just need to swap the 02 addresses
								romBuf[0xFFFF6C] = 0xF0;
								romBuf[0xFFFF6D] = 0x1E;
								
								romBuf[0xFFFF70] = 0x34;
								romBuf[0xFFFF71] = 0x1F;
							}
						}
						
						// Make Riya's intro VO group 1 instead of 3
						// this prevents it from being cut by 'Fight!' VO
						if(riya_vo_fix) {
							romBuf[0xBCBCF8] = 1;
							romBuf[0xBCBCFA] = 1;
						}
						
						// Force a specific engine sample rate
						if(engineRate) {
							romBuf[0x07E4AE] = 0x90 | engineRate; // game default 4 (13379 Hz)
						}
						
						// This removes the calls to Yujo no Zakeru! and Dream Tag Tournament! voices
						if(noTitle_vo) {
						/*
						 *	08BCB2B4 = konjikinogash! group addr
						 *	080666b2 = bl PlaySpecificSoundID = 000
						 *
						 *	08BCB2BC = yujoZakeru! group addr
						 *	0806562A = bl PlaySpecificSoundID = 001 (0x12D)
						 *
						 *	08BCB2C4 = dreamTagTourn! group addr
						 *	08065124 = bl PlaySpecificSoundID = 002 (0x12E)
						 */
							
							// Remove Konjiki no Gash Bell call
						//	romBuf[0x0666b2] = 0;
						//	romBuf[0x0666b3] = 0;
						//	romBuf[0x0666b4] = 0;
						//	romBuf[0x0666b5] = 0;
							
							// Remove Yujo no Zakeru call
							romBuf[0x06562A] = 0;
							romBuf[0x06562B] = 0;
							romBuf[0x06562C] = 0;
							romBuf[0x06562D] = 0;
							
							// Remove Dream Tag Tournament call
							romBuf[0x065124] = 0;
							romBuf[0x065125] = 0;
							romBuf[0x065126] = 0;
							romBuf[0x065127] = 0;
							
						#if 0
							// Move ZB! call to DTT!
						//	romBuf[0x065120] = 0x96; // 0x97 is the original value, it's left-shifted by 1
							
							// Move ZB! to Yujo no Zakeru!
							romBuf[0x065628] = 0xE6; // 0xE7 is the original value, it's added to 0x46
							
							// The call to 'Konjiki no Gash!' is also used for the bao zakeruga scene
							// so you can't remove it!
						#endif
						}
						
						if(engineChannels) {
							// max channels, game default 12
							romBuf[0x07E4AD] = 0xF0 | engineChannels;
							// The F in 0xF0 is the global volume, already at max
						}
						
					#if 0
						// Find engine sample rate, it's at 0x07E4AC!
						// MnB is 0x826FC = 3
						// EA is 0x29344 = 2
						// EA2 1.1 is 0x091FDC
						
						// FFIV is 0x0BA6E8
						// FFVI is 0x13324C
						if(1) {
							printf("Looking for engine sample rate...\n");
							int j = 0;
							for(j=0; j < (8*1024*1024); j+=4) {
								//if(((romBuf[j+2] & 0xF0) == 0x90) && (romBuf[j+3] == 0) && (romBuf[j+6] == 0)
								if(((romBuf[j]) == 0) && ((romBuf[j+1] & 0xF0) > 0x40) && ((romBuf[j+1] & 0xF) < 0xD) && ((romBuf[j+1] & 0xF) > 2) &&
								((romBuf[j+2] & 0xF) > 0) && ((romBuf[j+2] & 0xF) < 0xD) &&
								((romBuf[j+2] & 0xF0) > 0x40) && ((romBuf[j+2] & 0xF0) < 0xA0) && (romBuf[j+3] == 0)
								 )
						//		&& ((romBuf[j+6]) == 0) && ((romBuf[j+7]) == 3))
									printf("Found potential sample rate at 0x%X\n", j);
							}
						}
					#endif
					
					fclose(fb);
				}
				
				// ipatix's sound mixer fixes the constant audio pops
				if(hq_mixer) {
					FILE *fp = fopen("ASM/m4a_hq_mixer_dtt.bin", "rb");
					
					if(fp) {
						// get file size
						int posMixer = 0xFFBE40;
						int fileSz = 0;
						fseek(fp, 0, SEEK_END);
						fileSz = ftell(fp);
						
						fseek(fp, 0, SEEK_SET);
						fread(&romBuf[posMixer], 1, fileSz, fp);
						fclose(fp);
						
						// hook it up
						int iwram = 0x03006500;
						memcpy(&romBuf[0x7C99C], &posMixer, 3);
						memcpy(&romBuf[0x7C9A0], &iwram, 4);
						memcpy(&romBuf[0x7BD0C], &iwram, 4);
						
						// set Thumb bit
						romBuf[0x7C99C] += 1;
						romBuf[0x7BD0C] += 1;
						
						// this is from a mmio ptr, other implementations do this.
						// no idea what this does, but it makes the game work...
						romBuf[0x7C9A4] = 0x18;
						romBuf[0x7C9A5] = 2;
					} else
						printf("Couldn't find mixer binary!\n\n");
				}
				
				// Handles all sprite changes
				GFX_COPY(romBuf, langID);
				
				// NOTE: FUN_0807c9cc plays sfx/music argument is the id
				// example arg at ROM 0x1542C, and 0x1571E
				
				// To play longer sounds than normally possible it seems this value is related
				// ROM: 0x7D15C, OG instruction 6A30, if you reduce it from 256 to 166 (1030) it works!
				// But this isn't the way to do it, more testing is needed.
				
				//0x7D17E = some kind of sound buffer value, too low breaks playback
				
				//0xBCBD58 the 3s tied to these pointers mean to cut off voice if a new voice is played
				// but this only affects sound test.
				
				//oh, it's just as messy as in Zakeru2, but it's doable.
				
				//004 0xBCB2D4 = longest sound, length at 0xFA3264
				//absolute address for ptr to length 0xFA3264
				
				//213 at 0xFA4988 0x10 bytes is the playback time??
				// 0xFA49A0
				
				// VO dub
				// samplerate? 5 bytes later is the total size of the PCM data
				//0000 0000 0040 A400 0000 0000 302C 0000 0000 0000
				
				printf("\n");
				
				unsigned trackRepoints = 0x01000000;
				if(vo_replace && expandROM) {
					printf("Looking for VO to replace or repoint...\n\n");
					
					// Extend some sounds so replacing them is easier
					// #190 - Bari's Victory VO
					romBuf[0xFA4706] = 0xCF; // (og 0xFE) Remove auto-keyoff
					romBuf[0xFA470A] = 0xB0; // increase wait
					romBuf[0xFA470B] = 0xB1; // end track, (this is empty, padded space)
					
					
					uint8_t* voBuf = NULL;
					unsigned voSize = 0;
					int id = 0;
					char numberID[16] = {0};
					int orderFix = 0;
					for(id = 0; id < 214; ++id) {
						// Check for out of order voices (Tia/Keith)
						orderFix = 0;
						if((id > 21 && id < 29) || (id > 111 && id < 116))
							orderFix = 1;
						else if(id == 29) // Megumi!
							orderFix = -7;
						else if(id == 116) // Berun!
							orderFix = -4;
						
						if(langID == 2)
							sprintf(numberID, "VO/ES/%03d.bin", id + orderFix);
						else
							sprintf(numberID, "VO/EN/%03d.bin", id + orderFix);
						
						fb = fopen(numberID, "rb");
						if(fb == NULL)
							continue;
						
					//	printf("Show me path found: %s\n", numberID);
						
						// obtain size
						fseek(fb, 0, SEEK_END);
						voSize = ftell(fb);
						fseek(fb, 0, SEEK_SET);
						
					#if 0 // no longer needed
						if(voSize > 0xFFFF) {
							printf("Skipping really big file... 0x%02X\n", voSize);
							fclose(fb);
							continue;
						}
					#endif
						
						// copy to buffer
						voBuf = malloc(voSize);
						fread(voBuf, 1, voSize, fb);
						
						// flag if sound needs repointing
						bool repoint = false;
						if(voSize > arr_sizes[id])
							repoint = true;
						
					//	printf("Show me repoint: %d\n", repoint);
						
						// 2 methods to repoint, 1: expand ROM, 2: provide all sounds and realign 'em.
						// Since method 2 is risky, going with method 1
						if(repoint) {
							if(!expandROM) {
								printf("File data bigger than original, use --more-space flag.\n");
								if(voBuf)
									free(voBuf);
								continue;
							}
							
							// Check in case user supplied bigger files than possible
							if((trackRepoints + voSize + 0x14) > 0x1FFFFFF) {
								printf("Exceeded 32MB limit, skipping...\n");
								fclose(fb);
								if(voBuf)
									free(voBuf);
								continue;
							}
							
							// Update the pointer
							romBuf[0xBC9EC8+(id*0xC)] = trackRepoints;
							romBuf[0xBC9EC8+(id*0xC)+1] = trackRepoints >> 8;
							romBuf[0xBC9EC8+(id*0xC)+2] = trackRepoints >> 16;
							romBuf[0xBC9EC8+(id*0xC)+3] = 9;
							
							//Set header
							romBuf[trackRepoints] = 0;
							romBuf[trackRepoints + 1] = 0;
							romBuf[trackRepoints + 2] = 0;
							romBuf[trackRepoints + 3] = 0;
							romBuf[trackRepoints + 4] = (srate*1024); //A44000 pitch, implies 10512 Hz
							romBuf[trackRepoints + 5] = (srate*1024) >> 8;
							romBuf[trackRepoints + 6] = (srate*1024) >> 16; //AC4400 is for 11025 Hz, just like EA2
							romBuf[trackRepoints + 7] = (srate*1024) >> 24;
							romBuf[trackRepoints + 8] = 0;
							romBuf[trackRepoints + 9] = 0;
							romBuf[trackRepoints + 0xA] = 0;
							romBuf[trackRepoints + 0xB] = 0;
							
							// Set the size
							romBuf[trackRepoints + 0xC] = voSize;
							romBuf[trackRepoints + 0xD] = voSize >> 8;
							romBuf[trackRepoints + 0xE] = voSize >> 16;
							romBuf[trackRepoints + 0xF] = 0;
							
							// Now finish the job!
							memcpy(&romBuf[trackRepoints + 0x10], voBuf, voSize);
							
							printf("%03d.bin wrote and repointed to 0x%X!\n", id + orderFix, trackRepoints);
							
							// Increase for the next repoint
							trackRepoints += voSize + 0x10 + 4;
							
							// 32bit align it
							uint8_t adv = 0;
							while((trackRepoints+adv) % 4 != 0)
								++adv;
							trackRepoints += adv;
						}
						else {
							// pointer for the current sound
							uint32_t ptrCurrent = 0;
							ptrCurrent = romBuf[0xBC9EC8+(id*0xC)] | romBuf[0xBC9EC8+(id*0xC)+1] << 8
								| romBuf[0xBC9EC8+(id*0xC)+2] << 16;
							
							// Should never happen because it always expects the clean ROM
							if(romBuf[0xBC9EC8+(id*0xC) + 3] == 9) {
								ptrCurrent = ptrCurrent | 1 << 24;
								printf("Using hi expanded size.\n");
							}
							
							// want sound to loop? Thought as much
							//romBuf[ptrCurrent + 0x3] = 0x40;
							
							// Set the new pitch
							romBuf[ptrCurrent + 0x4] = (srate*1024);
							romBuf[ptrCurrent + 0x5] = (srate*1024) >> 8;
							romBuf[ptrCurrent + 0x6] = (srate*1024) >> 16;
							romBuf[ptrCurrent + 0x7] = (srate*1024) >> 24;
							
							// Set the new size
							romBuf[ptrCurrent + 0xC] = voSize;
							romBuf[ptrCurrent + 0xD] = voSize >> 8;
							
							// Now copy the new sound
							memcpy(&romBuf[ptrCurrent + 0x10], voBuf, voSize);
							
							// Interesting, while 0x10 is where data starts (look at 002)
							// it seems the last 4 bytes are still read, causing a pop
							// on replaced (not repointed) sounds.
						#if 1
							// Remove this once sounds are padded to 32 bits!
							romBuf[ptrCurrent + 0x10 + voSize]     = 0;
							romBuf[ptrCurrent + 0x10 + voSize + 1] = 0;
							romBuf[ptrCurrent + 0x10 + voSize + 2] = 0;
							romBuf[ptrCurrent + 0x10 + voSize + 3] = 0;
						#endif
							
							// Additionally, og sounds are all 32bit aligned
							// but ffmpeg doesn't pad them, maybe I should pad them here.
						#if 0
							printf("OG size: 0x%X, ", voSize);
							voSize = (voSize + 3) & ~3;
							printf("New size: 0x%X\n", voSize);
							romBuf[ptrCurrent + 0x10 + voSize + 3] = 0;
							
							// No, better to just make all sounds 32-bit aligned
						#endif
							
							// It's possible to further optimize the space for ZIP/7z compression
							// by storing the original sound's size and subtracting it from the new
							// and then zero out the bottom by the result.
							
							printf("%03d.bin wrote at 0x%X!\n", id + orderFix, ptrCurrent);
						}
						
						if(voBuf)
							free(voBuf);
						voBuf = NULL;
						fclose(fb);
					}
				}
				
				printf("\n");
				
				// Music replacements using streams
				if(music_replace && expandROM) {
					// Replacing music for streams is possible by using this:
					// BC00 BB4B BDXX BF40 BE7F CF3C7F B0B0B0B0 B2 AAAAAA0B
					// Means to play stream, specifically XX with stereo panning, default vol
					// and delay indefinitely by jumping to address AAAAAAAA0B
					
					// NOTE: Pausing the game (only possible during battles)
					// will cut the stream and not turn it on again.
					// To avoid this problem you can prevent pausing the track
					// by nop-ing the call here:
					// 78F026F8 = bl 0x0807CA98
					// rombuf[0x4A48] = 0;
					// rombuf[0x4A49] = 0;
					// rombuf[0x4A4A] = 0;
					// rombuf[0x4A4B] = 0;
					
					// Here's the thing tho, the entire code for pausing music in stages
					// is 16 bytes, that's enough to do something cool, like lowering volume
					// but that means we need to hook more code to set the volume back.
					
					// That won't work out, BUT I don't like the idea of making the regular
					// BGM also play when pausing, so I'm gonna do an asm inject to skip
					// pausing conditionally, first I will check if new music exists.
					
					
					printf("Looking for MUSIC to replace and repoint...\n\n");
					
					// Set sample rate of streamed music to 11025 Hz
					srate = 11025;
					
					uint8_t* voBuf = NULL;
					unsigned voSize = 0;
					int id = 0;
					char numberID[64] = {0};
					bool useLoop = false;
					bool needPauseFix = false;
					for(id = 0; id < 33; ++id) {
						// No need to support multilang
						sprintf(numberID, "VO/MUSIC/%02d.bin", id);
						
						fb = fopen(numberID, "rb");
						if(fb == NULL)
							continue;
						
						// if it's stage BGM, this fix
						if(id > 4 && id < 19)
							needPauseFix = true;
						
					//	printf("Show me path found: %s\n", numberID);
						
						// obtain size
						fseek(fb, 0, SEEK_END);
						voSize = ftell(fb);
						fseek(fb, 0, SEEK_SET);
						
						// check if the file starts with LOOP
						int LoopMagic = 0;
						useLoop = false;
						fread(&LoopMagic, 1, 4, fb);
						if(LoopMagic == 0x504F4F4C) { //le text for LOOP
							// Get the next u32 for the loop start sample
							fread(&LoopMagic, 1, 4, fb);
							
							// Adjust to new size
							voSize -= 8;
							
							useLoop = true;
							
							printf("Loop for track #%02d set to 0x%06X -- ", id,
								(LoopMagic & 0xFF) << 24 |
								((LoopMagic >> 8) & 0xFF) << 16 |
								((LoopMagic >> 16) & 0xFF) << 8 |
								(LoopMagic >> 24) & 0xFF);
						} else
							fseek(fb, 0, SEEK_SET);
						
						// prevent buffer overflow
						if((trackRepoints + voSize + 0x14 + 0x3C) > 0x1FFFFFF) {
							printf("Exceeded 32MB limit, skipping...\n");
							fclose(fb);
							continue;
						}
						
						// copy to buffer
						voBuf = malloc(voSize);
						fread(voBuf, 1, voSize, fb);
						
						// streamed music always needs to be repointed
						bool repoint = true;
						
						if(repoint) {
							// Update the main pointer
							romBuf[arr_songs[id]]   = trackRepoints;
							romBuf[arr_songs[id]+1] = trackRepoints >> 8;
							romBuf[arr_songs[id]+2] = trackRepoints >> 16;
							romBuf[arr_songs[id]+3] = 9; //i.e. (trackRepoints >> 24) + 1
							
							// Set some command
							romBuf[trackRepoints]   = 1;
							romBuf[trackRepoints+1] = 0;
							romBuf[trackRepoints+2] = 0x7F;
							romBuf[trackRepoints+3] = 0x80;
							
							// First pointer is sample
							uint32_t sampleCommands = trackRepoints + 0x10;
							romBuf[trackRepoints+4] = sampleCommands;
							romBuf[trackRepoints+5] = sampleCommands >> 8;
							romBuf[trackRepoints+6] = sampleCommands >> 16;
							romBuf[trackRepoints+7] = 9;
							
							// Second pointer is track
							sampleCommands = trackRepoints + 0x20;
							romBuf[trackRepoints+8]  = sampleCommands;
							romBuf[trackRepoints+9]  = sampleCommands >> 8;
							romBuf[trackRepoints+10] = sampleCommands >> 16;
							romBuf[trackRepoints+11] = 9;
							
							trackRepoints += 0x10;
							
							// Contains pointer to sample
							romBuf[trackRepoints] = 0;
							romBuf[trackRepoints+1] = 0x3C;
							romBuf[trackRepoints+2] = 0;
							romBuf[trackRepoints+3] = 0;
							
							// pointer to sample
							sampleCommands = trackRepoints + 0x10 + 0x20;
							romBuf[trackRepoints+4] = sampleCommands;
							romBuf[trackRepoints+5] = sampleCommands >> 8;
							romBuf[trackRepoints+6] = sampleCommands >> 16;
							romBuf[trackRepoints+7] = 9;
							
							// decay, atk, these are defaults
							romBuf[trackRepoints+8]  = 0xFF;
							romBuf[trackRepoints+9]  = 0;
							romBuf[trackRepoints+10] = 0xFF;
							romBuf[trackRepoints+11] = 0;
							
							trackRepoints += 0x10;
							
							// NOTE: The sappy format was reversed by Bregalad.
							
							// Track format I wrote to mimic a looped stream.
							// There may be a smaller way to do this.
							//BC00 BB4B BDXX BF40 BE7F CF3C7F B0B0B0B0 B2 AAAAAA0B
							romBuf[trackRepoints]    = 0xBC;
							romBuf[trackRepoints+1]  = 0;
							romBuf[trackRepoints+2]  = 0xBB; // tempo command
							romBuf[trackRepoints+3]  = 0x4B; // tempo arg 0x4B default
							romBuf[trackRepoints+4]  = 0xBD; // play sample command
							romBuf[trackRepoints+5]  = 0;    // this is the sample id
							romBuf[trackRepoints+6]  = 0xBF; // panning
							romBuf[trackRepoints+7]  = 0x40; // center
							romBuf[trackRepoints+8]  = 0xBE; // volume command
							romBuf[trackRepoints+9]  = 0x7F; // volume arg
							romBuf[trackRepoints+10] = 0xCF; // play track
							romBuf[trackRepoints+11] = 0x3C;
							romBuf[trackRepoints+12] = 0x7F;
							romBuf[trackRepoints+13] = 0xB0; // wait
							romBuf[trackRepoints+14] = 0xB0;
							romBuf[trackRepoints+15] = 0xB0;
							romBuf[trackRepoints+16] = 0xB0;
							romBuf[trackRepoints+17] = 0xB2; // repeat last wait indefinitely
							romBuf[trackRepoints+18] = (trackRepoints+16);
							romBuf[trackRepoints+19] = (trackRepoints+16) >> 8;
							romBuf[trackRepoints+20] = (trackRepoints+16) >> 16;
							romBuf[trackRepoints+21] = 9;
							
							trackRepoints += 0x20;
							
							// Set header
							romBuf[trackRepoints]     = 0;
							romBuf[trackRepoints + 1] = 0;
							romBuf[trackRepoints + 2] = 0;
							romBuf[trackRepoints + 3] = useLoop ? 0x40 : 0;
							romBuf[trackRepoints + 4] = (srate*1024);
							romBuf[trackRepoints + 5] = (srate*1024) >> 8; // pitch
							romBuf[trackRepoints + 6] = (srate*1024) >> 16; // AC4400 = pitch for 11025 Hz
							romBuf[trackRepoints + 7] = (srate*1024) >> 24;
							romBuf[trackRepoints + 8]   = LoopMagic >> 24; // loop start sample u32
							romBuf[trackRepoints + 9]   = LoopMagic >> 16;
							romBuf[trackRepoints + 0xA] = LoopMagic >> 8;
							romBuf[trackRepoints + 0xB] = LoopMagic;
							
							// Set the size
							romBuf[trackRepoints + 0xC] = voSize;
							romBuf[trackRepoints + 0xD] = voSize >> 8;
							romBuf[trackRepoints + 0xE] = voSize >> 16;
							romBuf[trackRepoints + 0xF] = 0;
							
							// Now finish the job!
							memcpy(&romBuf[trackRepoints + 0x10], voBuf, voSize);
							
							printf("%02d.bin wrote and repointed to 0x%X!\n", id, trackRepoints);
							
							// Increase for the next repoint
							trackRepoints += voSize + 0x10 + 4;
							
							// 32bit align it
							uint8_t adv = 0;
							while((trackRepoints+adv) % 4 != 0)
								++adv;
							trackRepoints += adv;
						}
						
						if(voBuf)
							free(voBuf);
						voBuf = NULL;
						fclose(fb);
					}
					
					// It would be better to check against a stage ID
					// if it's been replaced then this asm hack will be useful
					if(needPauseFix) {
						// Remove useless redundant code
						uint32_t val = 0;
						memcpy(&romBuf[0x4A3C], &val, 4);
						
						// Jump to the new code, le on PC messes up
						val = 0x46874800; //0x00488746
						memcpy(&romBuf[0x4A40], &val, 4);
						val = 0x08FFFE40; //0x40FEFF08
						memcpy(&romBuf[0x4A44], &val, 4);
						
						// If it's not this address, the asm must be repointed again
						memcpy(&romBuf[0xFFFE40], dontPauseStream_bin, sizeof(dontPauseStream_bin));
						
						printf("Applied pause hack for streamed music.\n");
					}
				}
				
				// Mute stage BGM only
				if(mute_music) {
					uint32_t noMusic = 0x08BCBD64; // no music
					memcpy(&romBuf[0xBCA9A4], &noMusic, 4); // 05 MienaiTsubasa looped
					memcpy(&romBuf[0xBCA9CC], &noMusic, 4); // 06 stage 2
					memcpy(&romBuf[0xBCA9AC], &noMusic, 4); // 07 stage 3
					memcpy(&romBuf[0xBCA9DC], &noMusic, 4); // 08 stage 4
					memcpy(&romBuf[0xBCA9B4], &noMusic, 4); // 09 stage 5
					memcpy(&romBuf[0xBCA9BC], &noMusic, 4); // 10 stage 6
					memcpy(&romBuf[0xBCA9C4], &noMusic, 4); // 11 stage 7
					memcpy(&romBuf[0xBCA9D4], &noMusic, 4); // 12 stage 8
					memcpy(&romBuf[0xBCA9E4], &noMusic, 4); // 13 stage 9
					memcpy(&romBuf[0xBCA9EC], &noMusic, 4); // 14 stage 10
					memcpy(&romBuf[0xBCA9F4], &noMusic, 4); // 15 stage 11
					memcpy(&romBuf[0xBCAA0C], &noMusic, 4); // 16 stage 12
					memcpy(&romBuf[0xBCA9FC], &noMusic, 4); // 17 stage 13
					memcpy(&romBuf[0xBCAA04], &noMusic, 4); // 18 stage 14
					
					// title screen mienai
					// 0xBCA954
				}
				
				// Create a new file for inserting the script
				fb = fopen(outName[0] == 0 ? "out_dtt.gba" : outName, "wb");
				fwrite(romBuf, 1, DTT_ROMSize, fb);
				fclose(fb);
				
				// display the internal ROM build date
				printf("ROM Build Date: %s\n\n", &romBuf[0xFC1278]);
				
				printf("Script size: %d bytes\n", s);
				
				// This doesn't take into account ASM enhancements or other repointed data
				if(!expandROM)
					printf("Available space after: %d bytes\n", ogFreeSpace - s);
				
				if(ptrTable)
					free(ptrTable);
				if(romBuf)
					free(romBuf);
				fclose(fp);
				break;
				
        }
		else if(strcmp(argv[i], "-d") == 0) // open ROM to extract script to a text file
        {
                ++i;   //skip arg
                x = 0; //reset parse
                //printf("\n");
                printf("Script Decoding Mode\n\n");
				
				int fileSz = 0;
				FILE *fp = fopen(argv[i], "rb");
				if(fp == NULL) {
					printf("Failed to open file!\n\nAre you casting the right spell, human?!\n\n");
					break;
				}
				
				// Decode should be jp by default
				if(!langChanged)
					langID = 0;
				
				// get file size
				fseek(fp, 0, SEEK_END);
				fileSz = ftell(fp);
				
				fseek(fp, 0, SEEK_SET);
				
				// Check if it's the ROM
				if(fileSz < (16*1024*1024)) {
					printf("Hey, this isn't the right ROM size!\n\n");
					fclose(fp);
					break;
				}
				
				// Seek to the start of strings
				//fseek(fp, 0x21B5AC, SEEK_SET);
				
				// Go to the pointer table for strings
				fseek(fp, 0xFAFA94, SEEK_SET);
				
				unsigned lines = 616;
				
				// Create a buffer to store the pointer list so we don't have to seek the file back n forth
				LinesData * ptrTable = (LinesData *) malloc((lines*4)+4);
				if(!ptrTable)
					printf("Could not allocate memory...\n");
				
				// Copy the pointer table to the buf
				fread(ptrTable, 4, lines, fp);
				
				//printf("SHOW AFTER READ: 0x%X\n", ptrTable[174].lineptr);
				
				// Create a file to write to
				FILE *fc = fopen(outName[0] == 0 ? "script_dec.txt" : outName, "w");
				
				// Write header info
				//fprintf(fc, "%s%s%s%s\n\nln: %d\n\n", "\xEF", "\xBB", "\xBF", decIntro, lines);
				fprintf(fc, "%s\n\nln: %d\n\n", decIntro, lines);
				
				//printf("Entering Danger Zone!\n");
				
				//printf("Entering: 0x%X !\n", ptrTable[0].lineptr & 0xFFFFFF);
				
				int s = 0;
				for(s = 0; s < lines; ++s) {
					// Seek to the start of each line
					fseek(fp, ptrTable[s].lineptr & 0xFFFFFF, SEEK_SET);
					
					fprintf(fc, "%d\n%s: ", s+1, LANG_ID[langID]);
					
					// Now convert the string until null is found
					int readCHR = 0;
					int c = 0;
					for(c = 0; c < fileSz; c+=2) {
						// Time to make some assumptions.
						// Characters seem to always be 2 bytes, while color codes are 3 bytes
						// therefore I'll just check for 2 bytes and if it starts with 0x25 it's a color switch
						// BUT watch out for LINEBREAKS, they start with 0x25 but are 2 bytes.
						// BUT ALSO working with LE here so I'm writing the values backwards.
						fread(&readCHR, 1, 2, fp);
						
						//if(readCHR == 0x22E0)
						//	printf("Read chr: 0x%X\n", readCHR);
						
						if(((readCHR & 0xF0) == 0x80) || ((readCHR & 0xF0) == 0x90)) { // fullwidth bank
							// Conversion hard!
							//fprintf(fc, "fx");
							
							//uint16_t val = (readCHR >> 8) | (readCHR << 8);
							uint16_t val = readCHR;
							
							// use shift-jis directly
							if(val == 0x4087)
								fprintf(fc, "\\HEART");
							else
								fwrite(&val, 1, 2, fc);
						}
						else if(readCHR == 0x5242) // linebreak
						//	fprintf(fc, "\\N");
							fprintf(fc, "BR");
						else if(readCHR == 0x464C)
							fprintf(fc, "LF");
						else if(readCHR == 0x4445)
							fprintf(fc, "ED");
						
						if(readCHR == 0) {
							fprintf(fc, "\n\n");
							break;
						}
					}
				}
				
				//printf("Outside Danger Zone!\n");
				
				fclose(fp);
				fclose(fc);
				if(ptrTable)
					free(ptrTable);
				
				break;
				
       }
	   else if(strcmp(argv[i], "-x") == 0)
	   {
			// Heyy this is a test
			++i;   //skip arg
            x = 0; //reset parse
            printf("Voice Extraction Mode\n\n");
			
			int fileSz = 0;
			FILE *fp = fopen(argv[i], "rb");
			if(fp == NULL) {
				printf("Failed to open file!\n\nAre you casting the right spell, human?!\n\n");
				break;
			}
			
			// get file size
			fseek(fp, 0, SEEK_END);
			fileSz = ftell(fp);
			
			fseek(fp, 0, SEEK_SET);
			
			// Check if it's the ROM
			if(fileSz < (16*1024*1024)) {
				printf("Hey, this isn't the right ROM size!\n\n");
				fclose(fp);
				break;
			}
			
			// seek to the pointer table, that's all we need to jump to a specific voice
			fseek(fp, 0xBC9EC8, SEEK_SET);
			
			int ptrPos = 0;
			unsigned ptr = 0;
			
			if(specificVoice > -1) {
				fseek(fp, 0xBC9EC8+(specificVoice*0xC), SEEK_SET);
				fread(&ptr, 1, 4, fp);
				printf("Your VOICE is at 0x%03X\n\n", ptr & 0xFFFFFF);
			}
			else {
				printf("Dumping all VO to files!\n");
				unsigned count = 0;
				uint8_t* voBuf = NULL;
			//	mkdir("/VO", 0777);
			//	mkdir("/VO/JP", 0777);
				mkdir("/VO");
				mkdir("/VO/JP");
				for(ptrPos = 0; ptrPos < 214; ++ptrPos) {
					char numberID[32] = {0};
					unsigned samples = 0;
					unsigned rate = 0; // Not 0xA4 for all 214?
					fseek(fp, 0xBC9EC8+(ptrPos*0xC), SEEK_SET);
					fread(&ptr, 1, 4, fp);
					fseek(fp, (ptr & 0xFFFFFF) + 0xC, SEEK_SET);
				//	printf("Show me: 0x%X\n", (ptr & 0xFFFFFF) + 0xC);
					fread(&samples, 1, 4, fp);
					fseek(fp, (ptr & 0xFFFFFF) + 0x10, SEEK_SET); //VO 5 implies it's 0x10 NOT 0x14
					
				#if 0
					// Report rate, result: all 214 voices have the same speed
					fseek(fp, (ptr & 0xFFFFFF) + 0x4, SEEK_SET);
					fread(&rate, 1, 4, fp);
					printf("Show me rate: 0x%04X\n", rate);
				#endif
					
					// check in case
					if(!samples)
						break;
					
				//	sprintf(numberID, "%03d_0x%02X.bin", count, samples);
					sprintf(numberID, "VO/JP/%03d.bin", count);
					
				#if 1
					// We are now at the start of the actual raw data
					voBuf = malloc(samples);
					fread(voBuf, 1, samples, fp);
					FILE* outfp = fopen(numberID, "wb");
					fwrite(voBuf, 1, samples, outfp);
					fclose(outfp);
				#else
					FILE* outfp = fopen("array_sizes.txt", "a");
					fprintf(outfp, "0x%04X, ", samples);
					fclose(outfp);
				#endif
					if(voBuf)
						free(voBuf);
					++count;
				}
			}
			
			fclose(fp);
			
			break;
	   }
	   ++i; //nice
      }
      //half is 7px, full is 14px
  //    if(!lineBroke && (halfLimit*7) + (fullLimit*14) > 154)
    //      printf("\nYou have exceeded the 1st dialogue box 154 pixel limit by %d pixels.\n", (halfLimit*7) + (fullLimit*14));

      //only if a linebreak happened
      //if(lineBroke && (halfLimit1*7) + (fullLimit1*14) > 154)
      //    printf("\nYou have exceeded the 1st dialogue box 154 pixel limit by %d pixels.\n", (halfLimit1*7) + (fullLimit1*14));
     // if(lineBroke && (halfLimit*7) + (fullLimit*14) > 154)
      //    printf("\nYou have exceeded the 2nd dialogue box 154 pixel limit by %d pixels.\n", (halfLimit*7) + (fullLimit*14));
  }
  else
  {
     printf("No data to convert.\n");
     printf("Use /help to list usage instructions.\n");
  }

  printf("\n");

  return 0;
}
