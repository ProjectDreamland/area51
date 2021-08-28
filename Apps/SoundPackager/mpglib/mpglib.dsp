# Microsoft Developer Studio Project File - Name="mpglib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=mpglib - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "mpglib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mpglib.mak" CFG="mpglib - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mpglib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "mpglib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "mpglib - Win32 Release NASM" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mpglib - Win32 Release"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "HAVE_CONFIG_H" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /I "../libmp3lame" /I "../include" /I ".." /D "NDEBUG" /D "HAVE_MPGLIB" /D "_WINDOWS" /D "USE_LAYER_2" /D "USE_LAYER_1" /D "WIN32" /D "HAVE_CONFIG_H" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "mpglib - Win32 Debug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "HAVE_CONFIG_H" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /Z7 /Od /I "../libmp3lame" /I "../include" /I ".." /D "_DEBUG" /D "_WINDOWS" /D "USE_LAYER_2" /D "USE_LAYER_1" /D "HAVE_MPGLIB" /D "WIN32" /D "HAVE_CONFIG_H" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "mpglib - Win32 Release NASM"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "mpglib__"
# PROP BASE Intermediate_Dir "mpglib__"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "mpglib__"
# PROP Intermediate_Dir "mpglib__"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /O2 /I "../libmp3lame" /I "../include" /I ".." /D "NDEBUG" /D "HAVE_MPGLIB" /D "_WINDOWS" /D "USE_LAYER_2" /D "USE_LAYER_1" /D "WIN32" /D "HAVE_CONFIG_H" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /W3 /O2 /I "../libmp3lame" /I "../include" /I ".." /D "NDEBUG" /D "HAVE_MPGLIB" /D "_WINDOWS" /D "USE_LAYER_2" /D "USE_LAYER_1" /D "WIN32" /D "HAVE_CONFIG_H" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Release_NASM\mpglib.lib"

!ENDIF 

# Begin Target

# Name "mpglib - Win32 Release"
# Name "mpglib - Win32 Debug"
# Name "mpglib - Win32 Release NASM"
# Begin Group "Source"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=.\common.c
# End Source File
# Begin Source File

SOURCE=.\dct64_i386.c
# End Source File
# Begin Source File

SOURCE=.\decode_i386.c
# End Source File
# Begin Source File

SOURCE=.\interface.c
# End Source File
# Begin Source File

SOURCE=.\layer1.c
# End Source File
# Begin Source File

SOURCE=.\layer2.c
# End Source File
# Begin Source File

SOURCE=.\layer3.c
# End Source File
# Begin Source File

SOURCE=.\tabinit.c
# End Source File
# End Group
# Begin Group "Include"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=..\configMS.h

!IF  "$(CFG)" == "mpglib - Win32 Release"

# Begin Custom Build
InputPath=..\configMS.h

"..\config.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy ..\configMS.h ..\config.h

# End Custom Build

!ELSEIF  "$(CFG)" == "mpglib - Win32 Debug"

# Begin Custom Build
InputPath=..\configMS.h

"..\config.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy ..\configMS.h ..\config.h

# End Custom Build

!ELSEIF  "$(CFG)" == "mpglib - Win32 Release NASM"

# Begin Custom Build
InputPath=..\configMS.h

"..\config.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy ..\configMS.h ..\config.h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dct64_i386.h
# End Source File
# Begin Source File

SOURCE=.\decode_i386.h
# End Source File
# Begin Source File

SOURCE=.\huffman.h
# End Source File
# Begin Source File

SOURCE=.\interface.h
# End Source File
# Begin Source File

SOURCE=.\layer1.h
# End Source File
# Begin Source File

SOURCE=.\layer2.h
# End Source File
# Begin Source File

SOURCE=.\layer3.h
# End Source File
# Begin Source File

SOURCE=.\mpg123.h
# End Source File
# Begin Source File

SOURCE=.\mpglib.h
# End Source File
# Begin Source File

SOURCE=.\tabinit.h
# End Source File
# End Group
# End Target
# End Project
