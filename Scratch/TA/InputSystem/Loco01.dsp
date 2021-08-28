# Microsoft Developer Studio Project File - Name="Loco01" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Loco01 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Loco01.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Loco01.mak" CFG="Loco01 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Loco01 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Loco01 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Loco01"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Loco01 - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "$(X)" /I "$(X)/x_Files" /I "$(X)/Entropy" /I "$(X)/MeshUtil" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "Loco01 - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(X)\Parsing" /I "$(X)" /I "$(X)/x_Files" /I "$(X)/Entropy" /I "$(X)/MeshUtil" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Loco01 - Win32 Release"
# Name "Loco01 - Win32 Debug"
# Begin Group "Source"

# PROP Default_Filter ""
# Begin Group "Animation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\Support\Animation\AnimData.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Support\Animation\AnimData.hpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Support\Animation\AnimPlayer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Support\Animation\AnimPlayer.hpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Support\Animation\AnimTrack.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Support\Animation\AnimTrack.hpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Support\Animation\CharAnimPlayer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Support\Animation\CharAnimPlayer.hpp
# End Source File
# End Group
# Begin Group "RscManager"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\Support\ResourceMgr\inline_ResourceMgr.hpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Support\ResourceMgr\ResourceMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Support\ResourceMgr\ResourceMgr.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\Support\InputMgr\InputMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Support\InputMgr\InputMgr.hpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\Auxiliary\MiscUtils\Property.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\Auxiliary\MiscUtils\Property.hpp
# End Source File
# End Group
# End Target
# End Project
