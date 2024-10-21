# Microsoft Developer Studio Project File - Name="Editor" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Editor - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Editor.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Editor.mak" CFG="Editor - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Editor - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Editor - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Editor"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Editor - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I "$(X)" /I "$(X)/x_Files" /I "$(X)/Entropy" /I "$(X)/MeshUtil" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /D "_WINDOWS" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "$(USERNAME)" /FR /YX"BaseStdAfx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "../AudioEditor" /i "../WinControls" /i "../WorldEditor" /i "../MeshViewer" /i "../EDRscDesc" /i "../EventEditor" /i "../LocoEditor" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 Shlwapi.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"libc.lib"

!ELSEIF  "$(CFG)" == "Editor - Win32 Debug"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(X)" /I "$(X)/x_Files" /I "$(X)/Entropy" /I "$(X)/MeshUtil" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "$(USERNAME)" /FR /YX"BaseStdAfx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "../AudioEditor" /i "../WinControls" /i "../WorldEditor" /i "../EventEditor" /i "../LocoEditor" /i "../MeshViewer" /i "../EDRscDesc" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Shlwapi.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcd.lib" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "Editor - Win32 Release"
# Name "Editor - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "h;hpp;cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "EditorBase"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Axis3d.cpp
# End Source File
# Begin Source File

SOURCE=.\Axis3d.hpp
# End Source File
# Begin Source File

SOURCE=.\BaseDocument.cpp
# End Source File
# Begin Source File

SOURCE=.\BaseDocument.h
# End Source File
# Begin Source File

SOURCE=.\BaseFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\BaseFrame.h
# End Source File
# Begin Source File

SOURCE=.\BaseStdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\BaseStdAfx.h
# End Source File
# Begin Source File

SOURCE=.\BaseView.cpp
# End Source File
# Begin Source File

SOURCE=.\BaseView.h
# End Source File
# Begin Source File

SOURCE=.\Grid3d.cpp
# End Source File
# Begin Source File

SOURCE=.\Grid3d.hpp
# End Source File
# Begin Source File

SOURCE=.\PaletteView.cpp
# End Source File
# Begin Source File

SOURCE=.\PaletteView.h
# End Source File
# Begin Source File

SOURCE=.\RealTimeMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\RealTimeMessage.h
# End Source File
# Begin Source File

SOURCE=.\View3D.cpp
# End Source File
# Begin Source File

SOURCE=.\View3D.h
# End Source File
# End Group
# Begin Group "MFCStuff"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BugBase.cpp
# End Source File
# Begin Source File

SOURCE=.\BugBase.h
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.h
# End Source File
# Begin Source File

SOURCE=.\DisableScreenSave.cpp
# End Source File
# Begin Source File

SOURCE=.\DisableScreenSave.h
# End Source File
# Begin Source File

SOURCE=.\Editor.cpp
# End Source File
# Begin Source File

SOURCE=.\Editor.h
# End Source File
# Begin Source File

SOURCE=.\HelpView.cpp
# End Source File
# Begin Source File

SOURCE=.\HelpView.h
# End Source File
# Begin Source File

SOURCE=.\LogView.cpp
# End Source File
# Begin Source File

SOURCE=.\LogView.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\MyRichEditCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\MyRichEditCtrl.h
# End Source File
# Begin Source File

SOURCE=.\OutputBar.cpp
# End Source File
# Begin Source File

SOURCE=.\OutputBar.h
# End Source File
# Begin Source File

SOURCE=.\ProjectDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\ProjectDoc.h
# End Source File
# Begin Source File

SOURCE=.\ProjectFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\ProjectFrame.h
# End Source File
# Begin Source File

SOURCE=.\ProjectView.cpp
# End Source File
# Begin Source File

SOURCE=.\ProjectView.h
# End Source File
# Begin Source File

SOURCE=.\ProjectViewFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\ProjectViewFrame.h
# End Source File
# Begin Source File

SOURCE=.\RscView.cpp
# End Source File
# Begin Source File

SOURCE=.\RscView.h
# End Source File
# Begin Source File

SOURCE=.\TabDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\TabDoc.h
# End Source File
# Begin Source File

SOURCE=.\TabView.cpp
# End Source File
# Begin Source File

SOURCE=.\TabView.h
# End Source File
# Begin Source File

SOURCE=.\TextEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\TextEditor.h
# End Source File
# Begin Source File

SOURCE=.\TextEditorView.cpp
# End Source File
# Begin Source File

SOURCE=.\TextEditorView.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\Support\AI\brain_stage_five.hpp
# End Source File
# Begin Source File

SOURCE=.\Lighting.cpp
# End Source File
# Begin Source File

SOURCE=.\Lighting.hpp
# End Source File
# Begin Source File

SOURCE=.\OutputCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\OutputCtrl.h
# End Source File
# Begin Source File

SOURCE=.\Project.cpp
# End Source File
# Begin Source File

SOURCE=.\Project.hpp
# End Source File
# Begin Source File

SOURCE=.\RaycastLighting.cpp
# End Source File
# Begin Source File

SOURCE=.\RaycastLighting.hpp
# End Source File
# Begin Source File

SOURCE=.\ResourceDesc.cpp
# End Source File
# Begin Source File

SOURCE=.\ResourceDesc.hpp
# End Source File
# Begin Source File

SOURCE=.\SlowLighting.cpp
# End Source File
# Begin Source File

SOURCE=.\SlowLighting.hpp
# End Source File
# Begin Source File

SOURCE=.\UserMessage.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\Editor.ico
# End Source File
# Begin Source File

SOURCE=.\Editor.rc
# End Source File
# Begin Source File

SOURCE=.\res\Editor.rc2
# End Source File
# Begin Source File

SOURCE=.\res\EditorBuildToolBar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\EditorDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\helpbraw.bmp
# End Source File
# Begin Source File

SOURCE=.\res\idr_proj.ico
# End Source File
# Begin Source File

SOURCE=.\res\ImgView.bmp
# End Source File
# Begin Source File

SOURCE=.\res\proj_ric.bmp
# End Source File
# Begin Source File

SOURCE=.\res\proj_too.bmp
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# Begin Group "Helpers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\xCore\3rdParty\Miles6\lib\win\Mss32.lib
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\res\web_thin.avi
# End Source File
# End Target
# End Project
