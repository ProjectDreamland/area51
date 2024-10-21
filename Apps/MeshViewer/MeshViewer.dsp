# Microsoft Developer Studio Project File - Name="MeshViewer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=MeshViewer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MeshViewer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MeshViewer.mak" CFG="MeshViewer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MeshViewer - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "MeshViewer - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "MeshViewer"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MeshViewer - Win32 Release"

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

!ELSEIF  "$(CFG)" == "MeshViewer - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(X)\Auxiliary\Bitmap" /I "$(X)" /I "$(X)/x_Files" /I "$(X)/Entropy" /I "$(X)/MeshUtil" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /D "_LIB" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "$(USERNAME)" /Yu"stdafx.h" /FD /GZ /c
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

# Name "MeshViewer - Win32 Release"
# Name "MeshViewer - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "h;hpp;cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\MeshViewer.cpp
# End Source File
# Begin Source File

SOURCE=.\MeshViewer.hpp
# End Source File
# Begin Source File

SOURCE=.\MeshViewer_Doc.cpp
# End Source File
# Begin Source File

SOURCE=.\MeshViewer_Doc.h
# End Source File
# Begin Source File

SOURCE=.\MeshViewer_Frame.cpp
# End Source File
# Begin Source File

SOURCE=.\MeshViewer_Frame.h
# End Source File
# Begin Source File

SOURCE=.\MeshViewer_View.cpp
# End Source File
# Begin Source File

SOURCE=.\MeshViewer_View.h
# End Source File
# Begin Source File

SOURCE=.\MeshWorkspaceDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\MeshWorkspaceDoc.h
# End Source File
# Begin Source File

SOURCE=.\MeshWorkspaceView.cpp
# End Source File
# Begin Source File

SOURCE=.\MeshWorkspaceView.h
# End Source File
# Begin Source File

SOURCE=.\RigidDesc.cpp
# End Source File
# Begin Source File

SOURCE=.\RigidDesc.hpp
# End Source File
# Begin Source File

SOURCE=.\SkinDesc.cpp
# End Source File
# Begin Source File

SOURCE=.\SkinDesc.hpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\res\MeshVtoolbar1.bmp
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\Script.rc
# End Source File
# End Group
# End Target
# End Project
