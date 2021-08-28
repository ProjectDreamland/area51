# Microsoft Developer Studio Project File - Name="~" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=~ - Win32 PS2 DevKit Debug IOP
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "~.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "~.mak" CFG="~ - Win32 PS2 DevKit Debug IOP"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "~ - Win32 PS2 DevKit Release IOP" (based on "Win32 (x86) Console Application")
!MESSAGE "~ - Win32 PS2 DevKit Debug IOP"   (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "~ - Win32 PS2 DevKit Release IOP"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE       Output_Dir "PS2-IOP-Release"
# PROP BASE Intermediate_Dir "PS2-IOP-Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP       Output_Dir "PS2-IOP-Release"
# PROP Intermediate_Dir "PS2-IOP-Release"
# PROP Target_Dir ""
# ADD BASE CPP /FD /O2 /D "TARGET_PS2_IOP" /D "VENDOR_SN"
# ADD      CPP /FD /O2 /D "TARGET_PS2_IOP" /D "VENDOR_SN"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD      RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD      BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /D:TARGET_PS2_IOP /D:VENDOR_SN /out:"PS2-IOP-Release\~.irx"
# ADD      LINK32 /D:TARGET_PS2_IOP /D:VENDOR_SN /out:"PS2-IOP-Release\~.irx"

!ELSEIF  "$(CFG)" == "~ - Win32 PS2 DevKit Debug IOP"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE       Output_Dir "PS2-IOP-Debug"
# PROP BASE Intermediate_Dir "PS2-IOP-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP       Output_Dir "PS2-IOP-Debug"
# PROP Intermediate_Dir "PS2-IOP-Debug"
# PROP Target_Dir ""
# ADD BASE CPP /FD /Zi /Od /D "TARGET_PS2_IOP" /D "VENDOR_SN"
# ADD      CPP /FD /Zi /Od /D "TARGET_PS2_IOP" /D "VENDOR_SN"
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD      RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD      BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /D:TARGET_PS2_IOP /D:VENDOR_SN /out:"PS2-IOP-Debug\~.irx"
# ADD      LINK32 /D:TARGET_PS2_IOP /D:VENDOR_SN /out:"PS2-IOP-Debug\~.irx"

!ENDIF 

# Begin Target

# Name "~ - Win32 PS2 DevKit Release IOP"
# Name "~ - Win32 PS2 DevKit Debug IOP"
# End Target
# End Project
