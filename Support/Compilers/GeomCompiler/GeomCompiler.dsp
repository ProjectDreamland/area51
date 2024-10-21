# Microsoft Developer Studio Project File - Name="GeomCompiler" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=GeomCompiler - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GeomCompiler.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GeomCompiler.mak" CFG="GeomCompiler - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GeomCompiler - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "GeomCompiler - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "GeomCompiler"
# PROP Scc_LocalPath "..\..\.."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GeomCompiler - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I ".\\" /I "$(X)\MeshUtil" /I "$(X)\Auxiliary\CommandLine" /I "$(X)\Parsing" /I "$(X)\..\Support" /D "NDEBUG" /D "$(USERNAME)" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "GeomCompiler - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I ".\\" /I "$(X)\MeshUtil" /I "$(X)\Auxiliary\CommandLine" /I "$(X)\Parsing" /I "$(X)\..\Support" /D "_DEBUG" /D "X_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "$(USERNAME)" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"C:\GameData\A51\Apps\Compilers_Dev/GeomCompiler.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "GeomCompiler - Win32 Release"
# Name "GeomCompiler - Win32 Debug"
# Begin Group "Nvidia"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\Nvidia\NvTriStrip\NvTriStrip.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\Nvidia\NvTriStrip\NvTriStrip.lib
# End Source File
# End Group
# Begin Group "Render"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Render\CollisionVolume.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Render\CollisionVolume.hpp
# End Source File
# Begin Source File

SOURCE=..\..\Render\geom.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Render\geom.hpp
# End Source File
# Begin Source File

SOURCE=..\..\Render\RigidGeom.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Render\RigidGeom.hpp
# End Source File
# Begin Source File

SOURCE=..\..\Render\SkinGeom.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Render\SkinGeom.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ArmOptimizer.cpp
# End Source File
# Begin Source File

SOURCE=.\ArmOptimizer.hpp
# End Source File
# Begin Source File

SOURCE=.\BMPUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\BMPUtil.hpp
# End Source File
# Begin Source File

SOURCE=.\faststrip.cpp
# End Source File
# Begin Source File

SOURCE=.\faststrip.hpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\Auxiliary\MiscUtils\fileio.hpp
# End Source File
# Begin Source File

SOURCE=.\GeomCompiler.cpp
# End Source File
# Begin Source File

SOURCE=.\GeomCompiler.hpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\Auxiliary\MiscUtils\Guid.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\Auxiliary\MiscUtils\Guid.hpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\Auxiliary\MiscUtils\mem_stream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\Auxiliary\MiscUtils\mem_stream.hpp
# End Source File
# Begin Source File

SOURCE=.\PS2SkinOptimizer.cpp
# End Source File
# Begin Source File

SOURCE=.\PS2SkinOptimizer.hpp
# End Source File
# Begin Source File

SOURCE=.\PS2strip.cpp
# End Source File
# Begin Source File

SOURCE=.\PS2strip.hpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\Auxiliary\MiscUtils\slist.hpp
# End Source File
# Begin Source File

SOURCE=.\texinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\texinfo.hpp
# End Source File
# End Target
# End Project
