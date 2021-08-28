# Microsoft Developer Studio Project File - Name="*SoundPkg*" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=*SoundPkg* - Win32 PS2 DevKit Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SoundPkg.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SoundPkg.mak" CFG="*SoundPkg* - Win32 PS2 DevKit Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "*SoundPkg* - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "*SoundPkg* - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "*SoundPkg* - Win32 PS2 DVD Release" (based on "Win32 (x86) Console Application")
!MESSAGE "*SoundPkg* - Win32 PS2 DVD Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "*SoundPkg* - Win32 PS2 Client Release" (based on "Win32 (x86) Console Application")
!MESSAGE "*SoundPkg* - Win32 PS2 Client Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "*SoundPkg* - Win32 PS2 DevKit Release" (based on "Win32 (x86) Console Application")
!MESSAGE "*SoundPkg* - Win32 PS2 DevKit Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "*SoundPkg* - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(X)\x_files" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "$(X)\x_files" /I "$(X)\entropy" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "*SoundPkg* - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(X)\x_files" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "X_DEBUG" /D "X_ASSERT" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(X)\x_files" /I "$(X)\entropy" /I "$(X)\entropy\ps2\iop\ineviop" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "X_DEBUG" /D "X_ASSERT" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "*SoundPkg* - Win32 PS2 DVD Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PS2-DVD-Release"
# PROP BASE Intermediate_Dir "PS2-DVD-Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PS2-DVD-Release"
# PROP Intermediate_Dir "PS2-DVD-Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /O2 /I "$(X)\x_files" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "$(X)\entropy" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86 /out:"PS2-DVD-Release/SoundPkg.elf" /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LINK32 /machine:IX86 /out:"PS2-DVD-Release/SoundPkg.elf" /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "*SoundPkg* - Win32 PS2 DVD Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PS2-DVD-Debug"
# PROP BASE Intermediate_Dir "PS2-DVD-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2-DVD-Debug"
# PROP Intermediate_Dir "PS2-DVD-Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /Zi /Od /I "$(X)\x_files" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\entropy" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /debug /machine:IX86 /out:"PS2-DVD-Debug/SoundPkg.elf" /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LINK32 /debug /machine:IX86 /out:"PS2-DVD-Debug/SoundPkg.elf" /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "*SoundPkg* - Win32 PS2 Client Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PS2-Client-Release"
# PROP BASE Intermediate_Dir "PS2-Client-Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PS2-Client-Release"
# PROP Intermediate_Dir "PS2-Client-Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /O2 /I "$(X)\x_files" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "$(X)\entropy" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86 /out:"PS2-Client-Release/SoundPkg.elf" /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LINK32 /machine:IX86 /out:"PS2-Client-Release/SoundPkg.elf" /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "*SoundPkg* - Win32 PS2 Client Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PS2-Client-Debug"
# PROP BASE Intermediate_Dir "PS2-Client-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2-Client-Debug"
# PROP Intermediate_Dir "PS2-Client-Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /Zi /Od /I "$(X)\x_files" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\entropy" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /debug /machine:IX86 /out:"PS2-Client-Debug/SoundPkg.elf" /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LINK32 /debug /machine:IX86 /out:"PS2-Client-Debug/SoundPkg.elf" /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "*SoundPkg* - Win32 PS2 DevKit Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PS2-DevKit-Release"
# PROP BASE Intermediate_Dir "PS2-DevKit-Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PS2-DevKit-Release"
# PROP Intermediate_Dir "PS2-DevKit-Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /O2 /I "$(X)\x_files" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "$(X)\entropy" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86 /out:"PS2-DevKit-Release/SoundPkg.elf" /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LINK32 /machine:IX86 /out:"PS2-DevKit-Release/SoundPkg.elf" /D:TARGET_PS2_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "*SoundPkg* - Win32 PS2 DevKit Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PS2-DevKit-Debug"
# PROP BASE Intermediate_Dir "PS2-DevKit-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2-DevKit-Debug"
# PROP Intermediate_Dir "PS2-DevKit-Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /Zi /Od /I "$(X)\x_files" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\entropy" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /debug /machine:IX86 /out:"PS2-DevKit-Debug/SoundPkg.elf" /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LINK32 /debug /machine:IX86 /out:"PS2-DevKit-Debug/SoundPkg.elf" /D:TARGET_PS2_DEV /D:VENDOR_SN

!ENDIF 

# Begin Target

# Name "*SoundPkg* - Win32 Release"
# Name "*SoundPkg* - Win32 Debug"
# Name "*SoundPkg* - Win32 PS2 DVD Release"
# Name "*SoundPkg* - Win32 PS2 DVD Debug"
# Name "*SoundPkg* - Win32 PS2 Client Release"
# Name "*SoundPkg* - Win32 PS2 Client Debug"
# Name "*SoundPkg* - Win32 PS2 DevKit Release"
# Name "*SoundPkg* - Win32 PS2 DevKit Debug"
# Begin Group "Headers"

# PROP Default_Filter "h;hpp"
# Begin Source File

SOURCE=.\AIFF.h
# End Source File
# Begin Source File

SOURCE=.\decode.hpp
# End Source File
# Begin Source File

SOURCE=.\encode.hpp
# End Source File
# Begin Source File

SOURCE=.\main.hpp
# End Source File
# Begin Source File

SOURCE=.\package.hpp
# End Source File
# Begin Source File

SOURCE=.\parse.hpp
# End Source File
# Begin Source File

SOURCE=.\parse_complex.hpp
# End Source File
# Begin Source File

SOURCE=.\parse_envelope.hpp
# End Source File
# Begin Source File

SOURCE=.\parse_variable.hpp
# End Source File
# End Group
# Begin Group "Source"

# PROP Default_Filter "cpp;c;s"
# Begin Source File

SOURCE=.\AIFF.cpp
# End Source File
# Begin Source File

SOURCE=.\decode.cpp
# End Source File
# Begin Source File

SOURCE=.\gcnEncode.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\package.cpp
# End Source File
# Begin Source File

SOURCE=.\parse.cpp
# End Source File
# Begin Source File

SOURCE=.\parse_complex.cpp
# End Source File
# Begin Source File

SOURCE=.\parse_envelope.cpp
# End Source File
# Begin Source File

SOURCE=.\parse_variable.cpp
# End Source File
# Begin Source File

SOURCE=.\pcEncode.cpp
# End Source File
# Begin Source File

SOURCE=.\ps2Encode.cpp
# End Source File
# End Group
# Begin Group "Libraries"

# PROP Default_Filter "lib"
# Begin Source File

SOURCE=.\Library\ENCVAG.LIB
# End Source File
# Begin Source File

SOURCE="..\..\..\..\..\Program Files\Microsoft Visual Studio\VC98\Lib\MSACM32.LIB"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\..\Program Files\Microsoft Visual Studio\VC98\Lib\WINMM.LIB"
# End Source File
# End Group
# End Target
# End Project
