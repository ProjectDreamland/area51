@echo off
echo Building files.dat....
if exist files.txt del /e /q /y files.txt
for %%i in (cdfs_log_*.txt) do type %%i >>files.txt
dir \gamedata\a51\release\ps2\*.* /s/b/a:-d >>files.txt
cdfsbuild files.txt
echo Building game executable...
copy /q ps2-dvd-release\gameapp.elf slus_201.49
C:\projects\a51\xCore\3rdParty\PS2\Sony\EE\bin\ee-strip.exe --strip-debug slus_201.49
echo Copying files to c:\temp\a51
if not exist c:\temp md c:\temp
if not exist c:\temp\a51 md c:\temp\a51
if not exist c:\temp\a51\modules md c:\temp\a51\modules
del /e /q /y c:\temp\a51 /s
copy /e /q \projects\a51\xcore\entropy\ps2\modules\*.irx c:\temp\a51\modules
copy /e /q \projects\a51\xcore\entropy\ps2\modules\*.img c:\temp\a51\modules
copy /e /q files.dat c:\temp\a51
copy /e /q slus_201.49 c:\temp\a51
copy /e /q system.cnf c:\temp\a51
copy /e /q A51-PS2-Disk-Config.ccz c:\temp
