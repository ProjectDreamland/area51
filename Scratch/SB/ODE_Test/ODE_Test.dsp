# Microsoft Developer Studio Project File - Name="*ODE_Test*" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=*ODE_Test* - Win32 PS2 DevKit Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ODE_Test.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ODE_Test.mak" CFG="*ODE_Test* - Win32 PS2 DevKit Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "*ODE_Test* - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "*ODE_Test* - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "*ODE_Test* - Win32 PS2 DVD Release" (based on "Win32 (x86) Application")
!MESSAGE "*ODE_Test* - Win32 PS2 DVD Debug" (based on "Win32 (x86) Application")
!MESSAGE "*ODE_Test* - Win32 PS2 Client Release" (based on "Win32 (x86) Application")
!MESSAGE "*ODE_Test* - Win32 PS2 Client Debug" (based on "Win32 (x86) Application")
!MESSAGE "*ODE_Test* - Win32 PS2 DevKit Release" (based on "Win32 (x86) Application")
!MESSAGE "*ODE_Test* - Win32 PS2 DevKit Debug" (based on "Win32 (x86) Application")
!MESSAGE "*ODE_Test* - Win32 GCN DevKit Release" (based on "Win32 (x86) Application")
!MESSAGE "*ODE_Test* - Win32 GCN DevKit Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "ODE_Test"
# PROP Scc_LocalPath "..\..\.."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "*ODE_Test* - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /I "$(X)\Parsing" /I "$(X)\3rdParty\ODE\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386

!ELSEIF  "$(CFG)" == "*ODE_Test* - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(X)\x_files" /I "$(X)\Entropy" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "X_DEBUG" /D "X_ASSERT" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /I "$(X)\Parsing" /I "$(X)\3rdParty\ODE\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "X_DEBUG" /D "X_ASSERT" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "*ODE_Test* - Win32 PS2 DVD Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PS2-DVD-Release"
# PROP BASE Intermediate_Dir "PS2-DVD-Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PS2-DVD-Release"
# PROP Intermediate_Dir "PS2-DVD-Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /I "$(X)\Parsing" /I "$(X)\3rdParty\ODE\include" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86 /out:"PS2-DVD-Release/Draw01.elf" /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LINK32 sntty.lib /machine:IX86 /out:"PS2-DVD-Release/Draw01.elf" /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "*ODE_Test* - Win32 PS2 DVD Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PS2-DVD-Debug"
# PROP BASE Intermediate_Dir "PS2-DVD-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2-DVD-Debug"
# PROP Intermediate_Dir "PS2-DVD-Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /I "$(X)\Parsing" /I "$(X)\3rdParty\ODE\include" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /debug /machine:IX86 /out:"PS2-DVD-Debug/Draw01.elf" /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LINK32 sntty.lib /debug /machine:IX86 /out:"PS2-DVD-Debug/Draw01.elf" /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "*ODE_Test* - Win32 PS2 Client Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PS2-Client-Release"
# PROP BASE Intermediate_Dir "PS2-Client-Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PS2-Client-Release"
# PROP Intermediate_Dir "PS2-Client-Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /I "$(X)\Parsing" /I "$(X)\3rdParty\ODE\include" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86 /out:"PS2-Client-Release/Draw01.elf" /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LINK32 sntty.lib /machine:IX86 /out:"PS2-Client-Release/Draw01.elf" /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "*ODE_Test* - Win32 PS2 Client Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PS2-Client-Debug"
# PROP BASE Intermediate_Dir "PS2-Client-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2-Client-Debug"
# PROP Intermediate_Dir "PS2-Client-Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /I "$(X)\Parsing" /I "$(X)\3rdParty\ODE\include" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /debug /machine:IX86 /out:"PS2-Client-Debug/Draw01.elf" /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LINK32 sntty.lib /debug /machine:IX86 /out:"PS2-Client-Debug/Draw01.elf" /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "*ODE_Test* - Win32 PS2 DevKit Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PS2-DevKit-Release"
# PROP BASE Intermediate_Dir "PS2-DevKit-Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PS2-DevKit-Release"
# PROP Intermediate_Dir "PS2-DevKit-Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /I "$(X)\Parsing" /I "$(X)\3rdParty\ODE\include" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86 /out:"PS2-DevKit-Release/Draw01.elf" /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LINK32 sntty.lib /machine:IX86 /out:"PS2-DevKit-Release/Draw01.elf" /D:TARGET_PS2_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "*ODE_Test* - Win32 PS2 DevKit Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PS2-DevKit-Debug"
# PROP BASE Intermediate_Dir "PS2-DevKit-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2-DevKit-Debug"
# PROP Intermediate_Dir "PS2-DevKit-Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /I "$(X)\Parsing" /I "$(X)\3rdParty\ODE\include" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /debug /machine:IX86 /out:"PS2-DevKit-Debug/Draw01.elf" /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LINK32 sntty.lib /debug /machine:IX86 /out:"PS2-DevKit-Debug/Draw01.elf" /D:TARGET_PS2_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "*ODE_Test* - Win32 GCN DevKit Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "GCN-DevKit-Release"
# PROP BASE Intermediate_Dir "GCN-DevKit-Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "GCN-DevKit-Release"
# PROP Intermediate_Dir "GCN-DevKit-Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_GCN_DEV" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /I "$(X)\Parsing" /I "$(X)\3rdParty\ODE\include" /D "TARGET_GCN_DEV" /D "VENDOR_SN" /D "HW2" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86 /out:"GCN-DevKit-Release/Draw01.elf" /D:TARGET_GCN_DEV /D:VENDOR_SN
# ADD LINK32 /machine:IX86 /out:"GCN-DevKit-Release/Draw01.elf" /D:TARGET_GCN_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "*ODE_Test* - Win32 GCN DevKit Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "GCN-DevKit-Debug"
# PROP BASE Intermediate_Dir "GCN-DevKit-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "GCN-DevKit-Debug"
# PROP Intermediate_Dir "GCN-DevKit-Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_GCN_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /I "$(X)\Parsing" /I "$(X)\3rdParty\ODE\include" /D "TARGET_GCN_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "HW2" /D "_DEBUG" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /debug /machine:IX86 /out:"GCN-DevKit-Debug/Draw01.elf" /D:TARGET_GCN_DEV /D:VENDOR_SN
# ADD LINK32 /debug /machine:IX86 /out:"GCN-DevKit-Debug/Draw01.elf" /D:TARGET_GCN_DEV /D:VENDOR_SN

!ENDIF 

# Begin Target

# Name "*ODE_Test* - Win32 Release"
# Name "*ODE_Test* - Win32 Debug"
# Name "*ODE_Test* - Win32 PS2 DVD Release"
# Name "*ODE_Test* - Win32 PS2 DVD Debug"
# Name "*ODE_Test* - Win32 PS2 Client Release"
# Name "*ODE_Test* - Win32 PS2 Client Debug"
# Name "*ODE_Test* - Win32 PS2 DevKit Release"
# Name "*ODE_Test* - Win32 PS2 DevKit Debug"
# Name "*ODE_Test* - Win32 GCN DevKit Release"
# Name "*ODE_Test* - Win32 GCN DevKit Debug"
# Begin Group "Support"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\crt0.s

!IF  "$(CFG)" == "*ODE_Test* - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "*ODE_Test* - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "*ODE_Test* - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "*ODE_Test* - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "*ODE_Test* - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "*ODE_Test* - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "*ODE_Test* - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "*ODE_Test* - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "*ODE_Test* - Win32 GCN DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "*ODE_Test* - Win32 GCN DevKit Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2.lk
# PROP Exclude_From_Scan -1
# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "ResourceMgr"

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
# Begin Group "MiscUtils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\xCore\Auxiliary\MiscUtils\fileio.hpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\Auxiliary\MiscUtils\Guid.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\Auxiliary\MiscUtils\Guid.hpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\Auxiliary\MiscUtils\mem_stream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\Auxiliary\MiscUtils\mem_stream.hpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\Auxiliary\MiscUtils\Property.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\Auxiliary\MiscUtils\Property.hpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\Auxiliary\MiscUtils\PropertyEnum.hpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\Auxiliary\MiscUtils\RTTI.hpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\Auxiliary\MiscUtils\slist.hpp
# End Source File
# End Group
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
# Begin Group "ODE"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\array.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\array.h
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\collision_kernel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\collision_kernel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\collision_space.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\collision_std.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\collision_std.h
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\collision_transform.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\collision_transform.h
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\collision_util.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\collision_util.h
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\include\ode\config.h
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\error.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\fastdot.c
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\fastldlt.c
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\fastlsolve.c
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\fastltsolve.c
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\geom.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\geom_internal.h
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\joint.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\joint.h
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\lcp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\lcp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\mass.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\mat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\mat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\matrix.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\memory.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\misc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\objects.h
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\obstack.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\obstack.h
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\ode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\odemath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\rotation.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\scrapbook.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\space.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\stack.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\stack.h
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\step.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\step.h
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\testing.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\testing.h
# End Source File
# Begin Source File

SOURCE=..\..\..\xCore\3rdParty\ODE\ode\src\timer.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Main.cpp
# End Source File
# End Target
# End Project
