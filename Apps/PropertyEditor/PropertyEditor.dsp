# Microsoft Developer Studio Project File - Name="PropertyEditor" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=PropertyEditor - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PropertyEditor.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PropertyEditor.mak" CFG="PropertyEditor - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PropertyEditor - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "PropertyEditor - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "PropertyEditor"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PropertyEditor - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /Zi /I "$(X)" /I "$(X)/x_Files" /I "$(X)/Entropy" /I "../Editor" /I "$(X)/MeshUtil" /I "$(X)\Auxiliary\Bitmap" /D "_LIB" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "$(USERNAME)" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "PropertyEditor - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(X)" /I "$(X)/x_Files" /I "$(X)/Entropy" /I "../Editor" /I "$(X)/MeshUtil" /I "$(X)\Auxiliary\Bitmap" /D "_LIB" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "$(USERNAME)" /Yu"stdafx.h" /FD /GZ /c
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

# Name "PropertyEditor - Win32 Release"
# Name "PropertyEditor - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Expression.cpp
# End Source File
# Begin Source File

SOURCE=.\Expression.h
# End Source File
# Begin Source File

SOURCE=.\GridComboBox.cpp
# End Source File
# Begin Source File

SOURCE=.\GridComboBox.h
# End Source File
# Begin Source File

SOURCE=.\GridEditCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\GridEditCtrl.h
# End Source File
# Begin Source File

SOURCE=.\GridItemInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\GridItemInfo.h
# End Source File
# Begin Source File

SOURCE=.\gridListctrl.cpp
# End Source File
# Begin Source File

SOURCE=.\gridListctrl.h
# End Source File
# Begin Source File

SOURCE=.\GridTreeItem.cpp
# End Source File
# Begin Source File

SOURCE=.\GridTreeItem.h
# End Source File
# Begin Source File

SOURCE=.\PropertyColumnSlider.cpp
# End Source File
# Begin Source File

SOURCE=.\PropertyColumnSlider.h
# End Source File
# Begin Source File

SOURCE=.\PropertyEditCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\PropertyEditCtrl.h
# End Source File
# Begin Source File

SOURCE=.\PropertyEditorDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\PropertyEditorDoc.h
# End Source File
# Begin Source File

SOURCE=.\PropertyEditorView.cpp
# End Source File
# Begin Source File

SOURCE=.\PropertyEditorView.h
# End Source File
# Begin Source File

SOURCE=.\PropertyGrid.cpp
# End Source File
# Begin Source File

SOURCE=.\PropertyGrid.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Expression_ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\Readme.txt
# End Source File
# End Target
# End Project
