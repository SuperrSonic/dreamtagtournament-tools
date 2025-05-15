# Dream Tag Tournament Tools

- Decodes the script into a text file.

- Encodes the text script into the game's format.

- Can replace voices to create a dub.

- Applies additional enhancements.


# Usage

Place the ROM in the same directory as dtt_conv.exe, name the ROM "DTT.gba" drag "script_dec.txt" into "patch_rom.cmd" and it will output "out_dtt.gba" with the script repointed and with a variable-width font.

Edit "patch_rom.cmd" to change language code.
You can also add --more-space to expand to 32MB, useful when combined with --vo-replace or --music-replace.

--mute-bgm will silence all stage music, --riya-fix will change Riya's intro VO to use group ID 1, to avoid it getting cut by the "Fight!" VO, --faudo-cheat will change the Faudo Race mini-game to use 'Easy' for all difficulties (it's terribly hard otherwise.) --op will make it so it's easy to gain max SP.

NOTE: If the base script isn't finished, untranslated lines will just be a garbled mess.


# Credits

Script tool "dtt_conv" and base translation by [SuperrSonic/Diego A.](https://www.youtube.com/@SuperrSonic)

English script revision, translations, and lore research by [XZatchBellGamerX/Raze](https://www.youtube.com/@XZatchBellGamerX)

Bregalad for his GBA sound player documentation.
