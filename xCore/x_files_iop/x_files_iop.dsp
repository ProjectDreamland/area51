# Microsoft Developer Studio Project File - Name="x_files_iop" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=x_files_iop - Win32 PS2 DevKit Debug IOP
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "x_files_iop.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "x_files_iop.mak" CFG="x_files_iop - Win32 PS2 DevKit Debug IOP"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "x_files_iop - Win32 PS2 DevKit Debug IOP" (based on "Win32 (x86) Static Library")
!MESSAGE "x_files_iop - Win32 PS2 DevKit Release IOP" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/A51/Scratch/bw/x_files_iop", SVDAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "x_files_iop - Win32 PS2 DevKit Debug IOP"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "x_files_iop___Win32_PS2_DevKit_Debug_IOP"
# PROP BASE Intermediate_Dir "x_files_iop___Win32_PS2_DevKit_Debug_IOP"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2-IOP-Debug"
# PROP Intermediate_Dir "PS2-IOP-Debug"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /Zi /Od /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /FD
# ADD CPP /Zi /Od /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /D "TARGET_PS2_IOP" /D "VENDOR_SN" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_IOP /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "x_files_iop - Win32 PS2 DevKit Release IOP"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x_files_iop___Win32_PS2_DevKit_Release_IOP"
# PROP BASE Intermediate_Dir "x_files_iop___Win32_PS2_DevKit_Release_IOP"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PS2-IOP-Release"
# PROP Intermediate_Dir "PS2-IOP-Release"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /O2 /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD CPP /O2 /D "TARGET_PS2_IOP" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_IOP /D:VENDOR_SN

!ENDIF 

# Begin Target

# Name "x_files_iop - Win32 PS2 DevKit Debug IOP"
# Name "x_files_iop - Win32 PS2 DevKit Release IOP"
# Begin Group "Implementation"

# PROP Default_Filter ""
# Begin Group "Inline"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Implementation\x_string_inline.hpp
# End Source File
# End Group
# Begin Group "Private"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Implementation\x_array_private.hpp
# End Source File
# Begin Source File

SOURCE=.\Implementation\x_files_private.hpp
# End Source File
# Begin Source File

SOURCE=.\Implementation\x_memory_private.hpp
# End Source File
# Begin Source File

SOURCE=.\Implementation\x_stdio_private.hpp
# End Source File
# End Group
# Begin Group "External"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\3rdParty\PS2\Sony\sce\ee\gcc\ee\include\_ansi.h
# End Source File
# Begin Source File

SOURCE=".\PS2-DevKit-Debug\_SN_in_VC.h"
# End Source File
# Begin Source File

SOURCE=..\3rdParty\PS2\Sony\sce\ee\gcc\ee\include\sys\config.h
# End Source File
# Begin Source File

SOURCE=..\3rdParty\PS2\Sony\sce\ee\gcc\ee\include\machine\ieeefp.h
# End Source File
# Begin Source File

SOURCE=..\3rdParty\PS2\Sony\sce\ee\gcc\ee\include\sys\reent.h
# End Source File
# Begin Source File

SOURCE=..\3rdParty\PS2\Sony\sce\ee\include\sifdev.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Implementation\x_bytestream.cpp
# End Source File
# Begin Source File

SOURCE=.\Implementation\x_debug.cpp
# End Source File
# Begin Source File

SOURCE=.\Implementation\x_files.cpp
# End Source File
# Begin Source File

SOURCE=.\Implementation\x_math_soft_float.cpp
# End Source File
# Begin Source File

SOURCE=.\Implementation\x_memory.cpp
# End Source File
# Begin Source File

SOURCE=.\Implementation\x_mqueue.cpp
# End Source File
# Begin Source File

SOURCE=.\Implementation\x_mutex.cpp
# End Source File
# Begin Source File

SOURCE=.\Implementation\x_plus.cpp
# End Source File
# Begin Source File

SOURCE=.\Implementation\x_profile.cpp
# End Source File
# Begin Source File

SOURCE=.\Implementation\x_stdio.cpp
# End Source File
# Begin Source File

SOURCE=.\Implementation\x_string.cpp
# End Source File
# Begin Source File

SOURCE=.\Implementation\x_threads.cpp
# End Source File
# Begin Source File

SOURCE=.\Implementation\x_time.cpp
# End Source File
# Begin Source File

SOURCE=.\Implementation\x_vsprintf.cpp
# End Source File
# End Group
# Begin Group "Under Construction"

# PROP Default_Filter ""
# End Group
# Begin Source File

SOURCE=.\x_array.hpp
# End Source File
# Begin Source File

SOURCE=.\x_bytestream.hpp
# End Source File
# Begin Source File

SOURCE=.\x_debug.hpp
# End Source File
# Begin Source File

SOURCE=.\x_files.hpp
# End Source File
# Begin Source File

SOURCE=.\Implementation\x_math_soft_float.hpp
# End Source File
# Begin Source File

SOURCE=.\x_memory.hpp
# End Source File
# Begin Source File

SOURCE=.\x_plus.hpp
# End Source File
# Begin Source File

SOURCE=.\x_profile.hpp
# End Source File
# Begin Source File

SOURCE=.\x_stdio.hpp
# End Source File
# Begin Source File

SOURCE=.\x_string.hpp
# End Source File
# Begin Source File

SOURCE=.\x_target.hpp
# End Source File
# Begin Source File

SOURCE=.\x_threads.hpp
# End Source File
# Begin Source File

SOURCE=.\x_time.hpp
# End Source File
# Begin Source File

SOURCE=.\x_types.hpp
# End Source File
# End Target
# End Project
