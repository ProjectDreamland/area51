# Microsoft Developer Studio Project File - Name="xbmpViewer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=xbmpViewer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "xbmpViewer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "xbmpViewer.mak" CFG="xbmpViewer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "xbmpViewer - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "xbmpViewer - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "xbmpViewer - Win32 Release"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /G6 /MT /W3 /GX /O2 /I "$(X)\x_files" /I "$(X)\auxiliary\bitmap" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /debug
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy release\xbmpViewer.exe c:\xBase\Tools
# End Special Build Tool

!ELSEIF  "$(CFG)" == "xbmpViewer - Win32 Debug"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(X)\x_files" /I "$(X)\auxiliary\bitmap" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "xbmpViewer - Win32 Release"
# Name "xbmpViewer - Win32 Debug"
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\hdrdown.bmp
# End Source File
# Begin Source File

SOURCE=.\res\hdrup.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ImgTab.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ImgView.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\xbmpViewer.ico
# End Source File
# Begin Source File

SOURCE=.\res\xbmpViewer.rc2
# End Source File
# End Group
# Begin Group "Framework"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\xbmpViewer.cpp
# End Source File
# Begin Source File

SOURCE=.\xbmpViewer.h
# End Source File
# Begin Source File

SOURCE=.\xbmpViewer.rc
# End Source File
# End Group
# Begin Group "Custom Bars"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BitmapViewer.cpp
# End Source File
# Begin Source File

SOURCE=.\BitmapViewer.h
# End Source File
# Begin Source File

SOURCE=.\ExplorerBar.cpp
# End Source File
# Begin Source File

SOURCE=.\ExplorerBar.h
# End Source File
# Begin Source File

SOURCE=.\FileList.cpp
# End Source File
# Begin Source File

SOURCE=.\FileList.h
# End Source File
# Begin Source File

SOURCE=.\ListViewCtrlEx.cpp
# End Source File
# Begin Source File

SOURCE=.\ListViewCtrlEx.h
# End Source File
# Begin Source File

SOURCE=.\PreviewBar.cpp
# End Source File
# Begin Source File

SOURCE=.\PreviewBar.h
# End Source File
# Begin Source File

SOURCE=.\Progress.cpp
# End Source File
# Begin Source File

SOURCE=.\Progress.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\xbmpViewer.exe.manifest
# End Source File
# End Target
# End Project
