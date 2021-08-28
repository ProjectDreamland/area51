# Microsoft Developer Studio Project File - Name="WorldEditor" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=WorldEditor - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "WorldEditor.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "WorldEditor.mak" CFG="WorldEditor - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WorldEditor - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "WorldEditor - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "WorldEditor"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "WorldEditor - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I "$(X)" /I "$(X)/x_Files" /I "$(X)/Entropy" /I "$(X)/MeshUtil" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /I "$(X)\Auxiliary\Bitmap" /D "_LIB" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "$(USERNAME)" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "WorldEditor - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(X)" /I "$(X)/x_Files" /I "$(X)/Entropy" /I "$(X)/MeshUtil" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /I "$(X)\Auxiliary\Bitmap" /D "_LIB" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "$(USERNAME)" /Yu"stdafx.h" /FD /GZ /c
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

# Name "WorldEditor - Win32 Release"
# Name "WorldEditor - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "SubPalette"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\EditorAIView.cpp
# End Source File
# Begin Source File

SOURCE=.\EditorAIView.h
# End Source File
# Begin Source File

SOURCE=.\EditorBlueprintView.cpp
# End Source File
# Begin Source File

SOURCE=.\EditorBlueprintView.h
# End Source File
# Begin Source File

SOURCE=.\EditorDebuggerView.cpp
# End Source File
# Begin Source File

SOURCE=.\EditorDebuggerView.h
# End Source File
# Begin Source File

SOURCE=.\EditorDecalView.cpp
# End Source File
# Begin Source File

SOURCE=.\EditorDecalView.h
# End Source File
# Begin Source File

SOURCE=.\EditorGlobalView.cpp
# End Source File
# Begin Source File

SOURCE=.\EditorGlobalView.h
# End Source File
# Begin Source File

SOURCE=.\EditorLayerView.cpp
# End Source File
# Begin Source File

SOURCE=.\EditorLayerView.h
# End Source File
# Begin Source File

SOURCE=.\EditorObjectView.cpp
# End Source File
# Begin Source File

SOURCE=.\EditorObjectView.h
# End Source File
# Begin Source File

SOURCE=.\EditorPaletteDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\EditorPaletteDoc.h
# End Source File
# Begin Source File

SOURCE=.\EditorSettingsView.cpp
# End Source File
# Begin Source File

SOURCE=.\EditorSettingsView.h
# End Source File
# Begin Source File

SOURCE=.\EditorSoundView.cpp
# End Source File
# Begin Source File

SOURCE=.\EditorSoundView.h
# End Source File
# Begin Source File

SOURCE=.\EditorTriggerView.cpp
# End Source File
# Begin Source File

SOURCE=.\EditorTriggerView.h
# End Source File
# Begin Source File

SOURCE=.\EditorWatchView.cpp
# End Source File
# Begin Source File

SOURCE=.\EditorWatchView.h
# End Source File
# Begin Source File

SOURCE=.\WorkspaceTabCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\WorkspaceTabCtrl.h
# End Source File
# End Group
# Begin Group "Dialogs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AnimPkgDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimPkgDialog.h
# End Source File
# Begin Source File

SOURCE=.\AudioPkgDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioPkgDialog.h
# End Source File
# Begin Source File

SOURCE=.\ConnectionAttributeDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ConnectionAttributeDialog.h
# End Source File
# Begin Source File

SOURCE=.\NewGlobalDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NewGlobalDlg.h
# End Source File
# Begin Source File

SOURCE=.\ResourceBrowserDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ResourceBrowserDlg.h
# End Source File
# Begin Source File

SOURCE=.\ResourcePreview.cpp
# End Source File
# Begin Source File

SOURCE=.\ResourcePreview.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\AI_Editor.cpp
# End Source File
# Begin Source File

SOURCE=.\AI_editor.hpp
# End Source File
# Begin Source File

SOURCE=.\BuildDFS.cpp
# End Source File
# Begin Source File

SOURCE=.\EditorDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\EditorDoc.h
# End Source File
# Begin Source File

SOURCE=.\EditorDocGridMngr.cpp
# End Source File
# Begin Source File

SOURCE=.\EditorDocGridMngr.h
# End Source File
# Begin Source File

SOURCE=.\EditorDocInputMngr.cpp
# End Source File
# Begin Source File

SOURCE=.\EditorDocInputMngr.h
# End Source File
# Begin Source File

SOURCE=.\EditorFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\EditorFrame.h
# End Source File
# Begin Source File

SOURCE=.\EditorView.cpp
# End Source File
# Begin Source File

SOURCE=.\EditorView.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\ValueGrid.cpp
# End Source File
# Begin Source File

SOURCE=.\ValueGrid.h
# End Source File
# Begin Source File

SOURCE=.\WE_Decals.cpp
# End Source File
# Begin Source File

SOURCE=.\WorldEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\WorldEditor.hpp
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\res\ai_view_toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\blueprin.bmp
# End Source File
# Begin Source File

SOURCE=.\res\decal_pal.bmp
# End Source File
# Begin Source File

SOURCE=.\res\global_f.bmp
# End Source File
# Begin Source File

SOURCE=.\res\layerlist_icons.bmp
# End Source File
# Begin Source File

SOURCE=.\res\layervie.bmp
# End Source File
# Begin Source File

SOURCE=.\res\obj_Pal.bmp
# End Source File
# Begin Source File

SOURCE=.\res\PAL_ImgTab.bmp
# End Source File
# Begin Source File

SOURCE=.\res\palettev_filter.bmp
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\res\SoundViewFilter.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tb_guidd.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tb_worldEditor.bmp
# End Source File
# Begin Source File

SOURCE=.\res\watchwindow.bmp
# End Source File
# Begin Source File

SOURCE=.\WorldEditor.rc
# End Source File
# End Group
# Begin Group "EditorObjs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\blueprint_anchor.cpp
# End Source File
# Begin Source File

SOURCE=.\blueprint_anchor.hpp
# End Source File
# Begin Source File

SOURCE=.\nav_connection2_anchor.cpp
# End Source File
# Begin Source File

SOURCE=.\nav_connection2_anchor.hpp
# End Source File
# Begin Source File

SOURCE=.\nav_connection2_editor.cpp
# End Source File
# Begin Source File

SOURCE=.\nav_connection2_editor.hpp
# End Source File
# Begin Source File

SOURCE=.\static_decal.cpp
# End Source File
# Begin Source File

SOURCE=.\static_decal.hpp
# End Source File
# End Group
# Begin Group "TransactionSystem"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ptr_list.cpp
# End Source File
# Begin Source File

SOURCE=.\ptr_list.hpp
# End Source File
# Begin Source File

SOURCE=.\transaction_bpref_data.cpp
# End Source File
# Begin Source File

SOURCE=.\transaction_bpref_data.hpp
# End Source File
# Begin Source File

SOURCE=.\transaction_data.hpp
# End Source File
# Begin Source File

SOURCE=.\transaction_entry.cpp
# End Source File
# Begin Source File

SOURCE=.\transaction_entry.hpp
# End Source File
# Begin Source File

SOURCE=.\transaction_file_data.cpp
# End Source File
# Begin Source File

SOURCE=.\transaction_file_data.hpp
# End Source File
# Begin Source File

SOURCE=.\transaction_layer_data.cpp
# End Source File
# Begin Source File

SOURCE=.\transaction_layer_data.hpp
# End Source File
# Begin Source File

SOURCE=.\transaction_mgr.cpp
# End Source File
# Begin Source File

SOURCE=.\transaction_mgr.hpp
# End Source File
# Begin Source File

SOURCE=.\transaction_object_data.cpp
# End Source File
# Begin Source File

SOURCE=.\transaction_object_data.hpp
# End Source File
# Begin Source File

SOURCE=.\transaction_selection_data.cpp
# End Source File
# Begin Source File

SOURCE=.\transaction_selection_data.hpp
# End Source File
# Begin Source File

SOURCE=.\transaction_zone_data.cpp
# End Source File
# Begin Source File

SOURCE=.\transaction_zone_data.hpp
# End Source File
# Begin Source File

SOURCE=.\TransactionStackDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\TransactionStackDlg.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Readme.txt
# End Source File
# Begin Source File

SOURCE=.\res\toolbar1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\triggerview.bmp
# End Source File
# End Target
# End Project
