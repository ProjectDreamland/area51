# Microsoft Developer Studio Project File - Name="AudioEditor" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=AudioEditor - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "AudioEditor.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AudioEditor.mak" CFG="AudioEditor - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AudioEditor - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "AudioEditor - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "AudioEditor"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "AudioEditor - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I "$(X)" /I "$(X)/x_Files" /I "$(X)/Entropy" /D "_LIB" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "$(USERNAME)" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "AudioEditor - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /D "_LIB" /D "$(USERNAME)" /D "WIN32" /D "_DEBUG" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
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

# Name "AudioEditor - Win32 Release"
# Name "AudioEditor - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "MFC"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\aiff_file.cpp
# End Source File
# Begin Source File

SOURCE=.\aiff_file.hpp
# End Source File
# Begin Source File

SOURCE=.\AudioDefines.h
# End Source File
# Begin Source File

SOURCE=.\AudioDirectoryView.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioDirectoryView.h
# End Source File
# Begin Source File

SOURCE=.\AudioEditorFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioEditorFrame.h
# End Source File
# Begin Source File

SOURCE=.\AudioEditorView.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioEditorView.h
# End Source File
# Begin Source File

SOURCE=.\DoubleBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\DoubleBuffer.h
# End Source File
# Begin Source File

SOURCE=.\ElementParamsDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ElementParamsDialog.h
# End Source File
# Begin Source File

SOURCE=.\FaderDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\FaderDialog.h
# End Source File
# Begin Source File

SOURCE=.\IntensityDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\IntensityDialog.h
# End Source File
# Begin Source File

SOURCE=.\IntensityView.cpp
# End Source File
# Begin Source File

SOURCE=.\IntensityView.h
# End Source File
# Begin Source File

SOURCE=.\ParamsDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ParamsDialog.h
# End Source File
# Begin Source File

SOURCE=.\sound_file.cpp
# End Source File
# Begin Source File

SOURCE=.\sound_file.hpp
# End Source File
# Begin Source File

SOURCE=.\SoundDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundDoc.h
# End Source File
# Begin Source File

SOURCE=.\SoundView.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundView.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TabCtrlViewFix.cpp
# End Source File
# Begin Source File

SOURCE=.\TabCtrlViewFix.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\AudioEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioEditor.hpp
# End Source File
# Begin Source File

SOURCE=.\AudioEditorPackage.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioEditorPackage.hpp
# End Source File
# End Group
# Begin Group "Resource"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\res\audio_treectrl_icons.bmp
# End Source File
# Begin Source File

SOURCE=.\AudioEditor.rc
# End Source File
# Begin Source File

SOURCE=.\res\AudioEditorToolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\AudioTool.ico
# End Source File
# Begin Source File

SOURCE=.\res\Package_toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\res\sound_Toolbar.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\res\audiotreedrive_icons.bmp
# End Source File
# Begin Source File

SOURCE=.\Readme.txt
# End Source File
# End Target
# End Project
