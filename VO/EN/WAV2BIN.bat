for %%f IN (*.bin) do (
rm %%~nf.bin
echo  Removing all bin files...
)

for %%f IN (WAVE/*.wav) do (
"ffmpeg.exe" -i "WAVE/%%f" -f s8 -ar 10512 -ac 1  "%%~nf.bin"
echo  Converting WAVE to RAW...
)
PAUSE