@echo off
nmake /f miles32.mak
if errorlevel 1 goto done
rem nmake /f milesp16.mak
:done
