@echo off

if not exist "C:\Program Files\Microsoft Visual Studio\VC98\Bin\_cl.exe" goto Done

copy "C:\Program Files\Microsoft Visual Studio\VC98\Bin\_cl.exe"    "C:\Program Files\Microsoft Visual Studio\VC98\Bin\cl.exe"
copy "C:\Program Files\Microsoft Visual Studio\VC98\Bin\_link.exe"  "C:\Program Files\Microsoft Visual Studio\VC98\Bin\link.exe"

:Done
