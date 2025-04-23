for %%A IN (*.bin) do (
"ffmpeg.exe" -f s8 -ar 11025 -i "%%~A" "WAVE/%%~A.wav"
echo  Converting DTT BIN to WAVE...
)
PAUSE