# Microsoft Developer Studio Project File - Name="InevIop" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=InevIop - Win32 PS2 DevKit Debug IOP
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ineviop.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ineviop.mak" CFG="InevIop - Win32 PS2 DevKit Debug IOP"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "InevIop - Win32 PS2 DevKit Release IOP" (based on "Win32 (x86) Console Application")
!MESSAGE "InevIop - Win32 PS2 DevKit Debug IOP" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/T2/Scratch/BW/ineviop", IKBAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "InevIop - Win32 PS2 DevKit Release IOP"

# PROP Output_Dir "PS2-IOP-Release"
# PROP Intermediate_Dir "PS2-IOP-Release"
# PROP Ignore_Export_Lib 0
# ADD CPP /O1 /I "$(X)/entropy" /D "TARGET_PS2_IOP" /D "VENDOR_SN" /D "X_RELEASE" /FD
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 inetctl.ilb inet.ilb usbd.ilb usbmload.ilb netcnf.ilb /machine:IX86 /out:"C:\Projects\T2\xCore\Entropy\PS2\modules\ineviop.irx" /D:TARGET_PS2_IOP /D:VENDOR_SN
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "InevIop - Win32 PS2 DevKit Debug IOP"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PS2-IOP-Debug"
# PROP BASE Intermediate_Dir "PS2-IOP-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2-IOP-Debug"
# PROP Intermediate_Dir "PS2-IOP-Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /Zi /Od /D "TARGET_PS2_IOP" /D "VENDOR_SN" /FD
# ADD CPP /Zi /Od /I "$(X)/entropy" /D "TARGET_PS2_IOP" /D "VENDOR_SN" /D "X_DEBUG" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_DEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86 /out:"PS2-IOP-Debug\InevIop.irx" /D:TARGET_PS2_IOP /D:VENDOR_SN /D:TARGET_PS2_IOP /D:VENDOR_SN
# ADD LINK32 inetctl.ilb inet.ilb usbd.ilb usbmload.ilb netcnf.ilb /machine:IX86 /out:"C:\Projects\A51\xCore\Entropy\PS2\modules\ineviop.irx" /D:TARGET_PS2_IOP /D:VENDOR_SN
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "InevIop - Win32 PS2 DevKit Release IOP"
# Name "InevIop - Win32 PS2 DevKit Debug IOP"
# Begin Group "Headers"

# PROP Default_Filter "h;hpp;i"
# Begin Source File

SOURCE=.\iopcomms.h
# End Source File
# Begin Source File

SOURCE=.\iopmain.h
# End Source File
# Begin Source File

SOURCE=.\iopmqueue.h
# End Source File
# Begin Source File

SOURCE=.\iopthreadpri.h
# End Source File
# Begin Source File

SOURCE=.\IOPTYPES.H
# End Source File
# End Group
# Begin Group "Source"

# PROP Default_Filter "c;cpp;s"
# Begin Source File

SOURCE=.\iopmain.c
# End Source File
# Begin Source File

SOURCE=.\iopmqueue.c
# End Source File
# End Group
# Begin Group "Audio Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\iopaud_cfx.h
# End Source File
# Begin Source File

SOURCE=.\iopaud_container.h
# End Source File
# Begin Source File

SOURCE=.\iopaud_defines.h
# End Source File
# Begin Source File

SOURCE=.\iopaud_element.h
# End Source File
# Begin Source File

SOURCE=.\iopaud_host.h
# End Source File
# Begin Source File

SOURCE=.\iopaud_hoststream.h
# End Source File
# Begin Source File

SOURCE=.\iopaud_spu.h
# End Source File
# Begin Source File

SOURCE=.\iopaud_stream.h
# End Source File
# Begin Source File

SOURCE=.\iopaud_voice.h
# End Source File
# Begin Source File

SOURCE=.\iopaudio.h
# End Source File
# End Group
# Begin Group "Audio Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\iopaud_cfx.c
# End Source File
# Begin Source File

SOURCE=.\iopaud_container.c
# End Source File
# Begin Source File

SOURCE=.\iopaud_element.c
# End Source File
# Begin Source File

SOURCE=.\iopaud_hoststream.c
# End Source File
# Begin Source File

SOURCE=.\iopaud_spu.c
# End Source File
# Begin Source File

SOURCE=.\iopaud_stream.c
# End Source File
# Begin Source File

SOURCE=.\iopaud_voice.c
# End Source File
# Begin Source File

SOURCE=.\iopaudio.c
# End Source File
# End Group
# End Target
# End Project
