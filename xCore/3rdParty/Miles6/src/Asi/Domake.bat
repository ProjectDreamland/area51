@echo off
nmake /f mp3dec32.mak
if errorlevel 1 goto done
if exist internal.mak nmake /f internal.mak
