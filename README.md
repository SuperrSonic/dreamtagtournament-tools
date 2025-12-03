# Dream Tag Tournament Tools

I wrote this program to handle the script of "Konjiki no Gash Bell!! Yujo no Zakeru Dream Tag Tournament" which I've translated as "Zatch Bell! Dream Tag Tournament," this game released originally in 2005 and was developed by Dimps, a lot of the developers worked on the Sonic Advance series and Kirby & the Amazing Mirror!


- Decodes the script into a text file.

- Encodes the text script into the game's format.

- Patching system to modify graphics in a clean way.

- Can replace voices to create a dub.

- Can replace the BGM with streamed audio.

- Applies additional enhancements.


# Usage

Place the ROM in the same directory as dtt_conv.exe, name the ROM "DTT.gba" drag "script_dec.txt" into "patch_rom.cmd" and it will output "out_dtt.gba" with the script repointed and with a variable-width font.

Edit "patch_rom.cmd" to change the language code.

You can also add --more-space to expand to 32MB, useful when combined with --vo-replace or --music-replace.

- --mute-bgm will silence all stage music.

- --riya-fix will change Riya's intro VO to use group ID 1, to avoid it getting cut by the "Fight!" VO
- --faudo-cheat will change the Faudo Race mini-game to use 'Easy' for all difficulties (it's terribly hard otherwise.)
- --op will make it so it's easy to gain max SP.
- --zagurzem X lets you specify the amount of seconds zagurzem will last on the opponent.
- --force-samplerate X lets you change the output frequency of the sound engine. 4 is the original (13379 Hz.)
- --force-intro-vo X enables an assembly patch that forces only 1 of the intro voices to play, P1 or P2, instead of both.
- --no-title-vo removes the last 2 voice calls on the title screen, since there's no dub for it.


NOTE: mGBA version 0.9.3 was the last stable release that had the faster HLE bios decompression, it improves this game by eliminating audio stutters that would occur when the game has to decompress a lot of data, such as the stage backgrounds.


# Credits

Script tool "dtt_conv", graphics, base translation and Spanish translation by [SuperrSonic/Diego A.](https://www.youtube.com/@SuperrSonic)

English script revision, translations, and lore research by [XZatchBellGamerX/Raze](https://www.youtube.com/@XZatchBellGamerX)

Bregalad for his GBA sound player documentation.
