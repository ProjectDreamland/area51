@echo off

if exist "C:\Program Files\Microsoft Visual Studio\VC98\Bin\_cl.exe" goto Update

:Install
copy "C:\Program Files\Microsoft Visual Studio\VC98\Bin\cl.exe"    "C:\Program Files\Microsoft Visual Studio\VC98\Bin\_cl.exe"
copy "C:\Program Files\Microsoft Visual Studio\VC98\Bin\cl.exe"    "C:\Program Files\Microsoft Visual Studio\VC98\Bin\Copy of cl.exe"
copy "C:\Program Files\Microsoft Visual Studio\VC98\Bin\link.exe"  "C:\Program Files\Microsoft Visual Studio\VC98\Bin\_link.exe"
copy "C:\Program Files\Microsoft Visual Studio\VC98\Bin\link.exe"  "C:\Program Files\Microsoft Visual Studio\VC98\Bin\Copy of link.exe"

:Update
del  "C:\Program Files\Microsoft Visual Studio\VC98\Bin\cl.exe"
del  "C:\Program Files\Microsoft Visual Studio\VC98\Bin\link.exe"
copy "xCL.exe" "C:\Program Files\Microsoft Visual Studio\VC98\Bin\CL.exe"
copy "xCL.exe" "C:\Program Files\Microsoft Visual Studio\VC98\Bin\LINK.exe"

:Done
