# Microsoft Developer Studio Project File - Name="*SoundPackager*" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=*SoundPackager* - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SoundPackager.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SoundPackager.mak" CFG="*SoundPackager* - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "*SoundPackager* - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "*SoundPackager* - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "SoundPackager"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "*SoundPackager* - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\entropy\Audio" /I "include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX"stdafx.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib /nologo /subsystem:console /debug /machine:I386 /out:"SoundPackager.exe"

!ELSEIF  "$(CFG)" == "*SoundPackager* - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(X)\entropy\Audio" /I "$(X)" /I "$(X)\x_files" /I "include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "*SoundPackager* - Win32 Release"
# Name "*SoundPackager* - Win32 Debug"
# Begin Group "ADPCM"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\imaadpcm.cpp
# End Source File
# Begin Source File

SOURCE=.\imaadpcm.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\aiff_file.cpp
# End Source File
# Begin Source File

SOURCE=.\aiff_file.hpp
# End Source File
# Begin Source File

SOURCE=.\audio_private_pkg.hpp
# End Source File
# Begin Source File

SOURCE=.\Compress.hpp
# End Source File
# Begin Source File

SOURCE=.\Compress_GCN.cpp
# End Source File
# Begin Source File

SOURCE=.\Compress_PC.cpp
# End Source File
# Begin Source File

SOURCE=.\Compress_PS2.cpp
# End Source File
# Begin Source File

SOURCE=.\Compress_XBOX.cpp
# End Source File
# Begin Source File

SOURCE=.\DspTool.cpp
# End Source File
# Begin Source File

SOURCE=.\DspTool.hpp
# End Source File
# Begin Source File

SOURCE=.\ENCVAG.DLL
# End Source File
# Begin Source File

SOURCE=.\ENCVAG.H
# End Source File
# Begin Source File

SOURCE=.\endian.cpp
# End Source File
# Begin Source File

SOURCE=.\endian.hpp
# End Source File
# Begin Source File

SOURCE=.\ExportPackage.cpp
# End Source File
# Begin Source File

SOURCE=.\ExportPackage.hpp
# End Source File
# Begin Source File

SOURCE=.\PackageTypes.hpp
# End Source File
# Begin Source File

SOURCE=.\ParseScript.cpp
# End Source File
# Begin Source File

SOURCE=.\ParseScript.hpp
# End Source File
# Begin Source File

SOURCE=.\ProcessSample.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundPackager.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundPackager.hpp
# End Source File
# Begin Source File

SOURCE=.\test.txt
# End Source File
# Begin Source File

SOURCE=.\ENCVAG.LIB
# End Source File
# Begin Source File

SOURCE=..\..\xCore\3rdParty\Miles6\lib\win\Mss32.lib
# End Source File
# End Target
# End Project
