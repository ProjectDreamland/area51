# Microsoft Developer Studio Project File - Name="AnimCompiler" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=AnimCompiler - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "AnimCompiler.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AnimCompiler.mak" CFG="AnimCompiler - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AnimCompiler - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "AnimCompiler - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "AnimCompiler"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "AnimCompiler - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "C:\GameData\A51\Apps\Compilers_Dev"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /I "$(X)\.." /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I ".\\" /I "$(X)\MeshUtil" /I "$(X)\Auxiliary\CommandLine" /I "$(X)\..\Support\Animation" /I "$(X)\..\Support\ResourceMgr" /I "$(X)\Auxiliary\MiscUtils" /I "$(X)\..\Support" /I "$(X)\..\Support\Render" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "ANIM_COMPILER" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386

!ELSEIF  "$(CFG)" == "AnimCompiler - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "C:\GameData\A51\Apps\Compilers_Dev"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(X)\.." /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I ".\\" /I "$(X)\MeshUtil" /I "$(X)\Auxiliary\CommandLine" /I "$(X)\..\Support\Animation" /I "$(X)\..\Support\ResourceMgr" /I "$(X)\Auxiliary\MiscUtils" /I "$(X)\..\Support" /I "$(X)\..\Support\Render" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "ANIM_COMPILER" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "AnimCompiler - Win32 Release"
# Name "AnimCompiler - Win32 Debug"
# Begin Group "ResourceMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\ResourceMgr\inline_ResourceMgr.hpp
# End Source File
# Begin Source File

SOURCE=..\..\ResourceMgr\ResourceMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\ResourceMgr\ResourceMgr.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\AnimCompiler.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimCompiler.hpp
# End Source File
# Begin Source File

SOURCE=..\..\Animation\AnimCompress.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Animation\AnimData.hpp
# End Source File
# Begin Source File

SOURCE=..\..\Animation\AnimDecompress.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Animation\AnimEvent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Animation\AnimGroup.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Animation\AnimInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Animation\AnimKeyData.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# End Target
# End Project
