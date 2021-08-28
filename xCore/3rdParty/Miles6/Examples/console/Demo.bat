@echo off
%1
cd %2 >nul
cls
echo Welcome to the Miles Sound System for DOS!
echo ÿ
echo First, we'll demo the digital playback examples.
pause
cls
digplay ..\media\glass.wav
pause
cls
stream ..\media\wrong.mp3
pause
cls
sstest
pause
cls

echo Now, we'll try the MIDI examples.
pause
cls
dlsplay ..\media\demo.xmi
pause
cls
xmiplay ..\media\demo.xmi
pause
cls

echo Now, we'll try the quick API examples.
pause
cls
play ..\media\wrong.mp3
pause
cls
play ..\media\demo.xmi
cls

echo Finally, we'll try the redbook CD audio example.
pause
cls
audiocd
cls
echo All done!

