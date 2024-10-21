# Microsoft Developer Studio Project File - Name="WinControls" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=WinControls - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "WinControls.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "WinControls.mak" CFG="WinControls - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WinControls - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "WinControls - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "WinControls"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "WinControls - Win32 Release"

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
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I "$(X)" /I "$(X)/x_Files" /I "$(X)/Entropy" /I "../Editor" /D "_LIB" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "$(USERNAME)" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "WinControls - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(X)" /I "$(X)/x_Files" /I "$(X)/Entropy" /I "../Editor" /D "_LIB" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "$(USERNAME)" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "WinControls - Win32 Release"
# Name "WinControls - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\FileSearch.cpp
# End Source File
# Begin Source File

SOURCE=.\FileSearch.h
# End Source File
# Begin Source File

SOURCE=.\FileTreeCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\FileTreeCtrl.h
# End Source File
# Begin Source File

SOURCE=.\ListBoxDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ListBoxDlg.h
# End Source File
# Begin Source File

SOURCE=.\StackListBox.cpp
# End Source File
# Begin Source File

SOURCE=.\StackListBox.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\StringEntryDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StringEntryDlg.h
# End Source File
# Begin Source File

SOURCE=.\XTDialogToolBar.cpp
# End Source File
# Begin Source File

SOURCE=.\XTDialogToolBar.h
# End Source File
# Begin Source File

SOURCE=.\XTSortListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\XTSortListCtrl.h
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\res\tfdropcopy.cur
# End Source File
# Begin Source File

SOURCE=.\res\tfnodropcopy.cur
# End Source File
# Begin Source File

SOURCE=.\res\tfnodropmove.cur
# End Source File
# Begin Source File

SOURCE=.\res\treectrl_icons.bmp
# End Source File
# Begin Source File

SOURCE=.\WinControls.rc
# End Source File
# End Group
# Begin Source File

SOURCE=.\Readme.txt
# End Source File
# End Target
# End Project
