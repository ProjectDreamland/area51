# Microsoft Developer Studio Project File - Name="*~*" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=*~* - Win32 PS2 DevKit Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "~.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "~.mak" CFG="*~* - Win32 PS2 DevKit Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "*~* - Win32 Release"            (based on "Win32 (x86) Console Application")
!MESSAGE "*~* - Win32 Debug"              (based on "Win32 (x86) Console Application")
!MESSAGE "*~* - Win32 PS2 DVD Release"    (based on "Win32 (x86) Console Application")
!MESSAGE "*~* - Win32 PS2 DVD Debug"      (based on "Win32 (x86) Console Application")
!MESSAGE "*~* - Win32 PS2 Client Release" (based on "Win32 (x86) Console Application")
!MESSAGE "*~* - Win32 PS2 Client Debug"   (based on "Win32 (x86) Console Application")
!MESSAGE "*~* - Win32 PS2 DevKit Release" (based on "Win32 (x86) Console Application")
!MESSAGE "*~* - Win32 PS2 DevKit Debug"   (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "*~* - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE       Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP       Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(X)\x_files" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD      CPP /nologo /W3 /GX /O2 /I "$(X)\x_files" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD      RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD      BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD      LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "*~* - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE       Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP       Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(X)\x_files" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c /D "X_DEBUG" /D "X_ASSERT"
# ADD      CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(X)\x_files" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c /D "X_DEBUG" /D "X_ASSERT"
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD      RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD      BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD      LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "*~* - Win32 PS2 DVD Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE       Output_Dir "PS2-DVD-Release"
# PROP BASE Intermediate_Dir "PS2-DVD-Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP       Output_Dir "PS2-DVD-Release"
# PROP Intermediate_Dir "PS2-DVD-Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /FD /O2 /I "$(X)\x_files" /D "TARGET_PS2_DVD" /D "VENDOR_SN"
# ADD      CPP /FD /O2 /I "$(X)\x_files" /D "TARGET_PS2_DVD" /D "VENDOR_SN"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD      RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD      BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /D:TARGET_PS2_DVD /D:VENDOR_SN /out:"PS2-DVD-Release/~.elf"
# ADD      LINK32 /D:TARGET_PS2_DVD /D:VENDOR_SN /out:"PS2-DVD-Release/~.elf"

!ELSEIF  "$(CFG)" == "*~* - Win32 PS2 DVD Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE       Output_Dir "PS2-DVD-Debug"
# PROP BASE Intermediate_Dir "PS2-DVD-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP       Output_Dir "PS2-DVD-Debug"
# PROP Intermediate_Dir "PS2-DVD-Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /FD /Zi /Od /I "$(X)\x_files" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT"
# ADD      CPP /FD /Zi /Od /I "$(X)\x_files" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT"
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD      RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD      BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /debug /D:TARGET_PS2_DVD /D:VENDOR_SN /out:"PS2-DVD-Debug/~.elf"
# ADD      LINK32 /debug /D:TARGET_PS2_DVD /D:VENDOR_SN /out:"PS2-DVD-Debug/~.elf"

!ELSEIF  "$(CFG)" == "*~* - Win32 PS2 Client Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE       Output_Dir "PS2-Client-Release"
# PROP BASE Intermediate_Dir "PS2-Client-Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP       Output_Dir "PS2-Client-Release"
# PROP Intermediate_Dir "PS2-Client-Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /FD /O2 /I "$(X)\x_files" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN"
# ADD      CPP /FD /O2 /I "$(X)\x_files" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD      RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD      BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN /out:"PS2-Client-Release/~.elf"
# ADD      LINK32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN /out:"PS2-Client-Release/~.elf"

!ELSEIF  "$(CFG)" == "*~* - Win32 PS2 Client Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE       Output_Dir "PS2-Client-Debug"
# PROP BASE Intermediate_Dir "PS2-Client-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP       Output_Dir "PS2-Client-Debug"
# PROP Intermediate_Dir "PS2-Client-Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /FD /Zi /Od /I "$(X)\x_files" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT"
# ADD      CPP /FD /Zi /Od /I "$(X)\x_files" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT"
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD      RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD      BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /debug /D:TARGET_PS2_CLIENT /D:VENDOR_SN /out:"PS2-Client-Debug/~.elf"
# ADD      LINK32 /debug /D:TARGET_PS2_CLIENT /D:VENDOR_SN /out:"PS2-Client-Debug/~.elf"

!ELSEIF  "$(CFG)" == "*~* - Win32 PS2 DevKit Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE       Output_Dir "PS2-DevKit-Release"
# PROP BASE Intermediate_Dir "PS2-DevKit-Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP       Output_Dir "PS2-DevKit-Release"
# PROP Intermediate_Dir "PS2-DevKit-Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /FD /O2 /I "$(X)\x_files" /D "TARGET_PS2_DEV" /D "VENDOR_SN"
# ADD      CPP /FD /O2 /I "$(X)\x_files" /D "TARGET_PS2_DEV" /D "VENDOR_SN"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD      RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD      BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /D:TARGET_PS2_DEV /D:VENDOR_SN /out:"PS2-DevKit-Release/~.elf"
# ADD      LINK32 /D:TARGET_PS2_DEV /D:VENDOR_SN /out:"PS2-DevKit-Release/~.elf"

!ELSEIF  "$(CFG)" == "*~* - Win32 PS2 DevKit Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE       Output_Dir "PS2-DevKit-Debug"
# PROP BASE Intermediate_Dir "PS2-DevKit-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP       Output_Dir "PS2-DevKit-Debug"
# PROP Intermediate_Dir "PS2-DevKit-Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /FD /Zi /Od /I "$(X)\x_files" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT"
# ADD      CPP /FD /Zi /Od /I "$(X)\x_files" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT"
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD      RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD      BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /debug /D:TARGET_PS2_DEV /D:VENDOR_SN /out:"PS2-DevKit-Debug/~.elf"
# ADD      LINK32 /debug /D:TARGET_PS2_DEV /D:VENDOR_SN /out:"PS2-DevKit-Debug/~.elf"

!ENDIF 

# Begin Target

# Name "*~* - Win32 Release"
# Name "*~* - Win32 Debug"
# Name "*~* - Win32 PS2 DVD Release"
# Name "*~* - Win32 PS2 DVD Debug"
# Name "*~* - Win32 PS2 Client Release"
# Name "*~* - Win32 PS2 Client Debug"
# Name "*~* - Win32 PS2 DevKit Release"
# Name "*~* - Win32 PS2 DevKit Debug"

# Begin Source File
SOURCE=.\crt0.s
!IF  "$(CFG)" == "*~* - Win32 Release"
# PROP Exclude_From_Build 1
!ELSEIF  "$(CFG)" == "*~* - Win32 Debug"
# PROP Exclude_From_Build 1
!ENDIF 
# End Source File

# Begin Source File
SOURCE=.\PS2.lk
# PROP Exclude_From_Scan -1
# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1
# End Source File

# End Target
# End Project
