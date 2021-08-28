# Microsoft Developer Studio Project File - Name="Parsing" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Parsing - Win32 Xbox Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Parsing.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Parsing.mak" CFG="Parsing - Win32 Xbox Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Parsing - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Parsing - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Parsing - Win32 PS2 DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Parsing - Win32 PS2 DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Parsing - Win32 PS2 Client Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Parsing - Win32 PS2 Client Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Parsing - Win32 PS2 DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Parsing - Win32 PS2 DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Parsing - Win32 GCN DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Parsing - Win32 GCN DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Parsing - Win32 GCN DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Parsing - Win32 GCN DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Parsing - Win32 Xbox Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Parsing - Win32 Xbox Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Parsing"
# PROP Scc_LocalPath "..\.."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Parsing - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PC-Release"
# PROP BASE Intermediate_Dir "PC-Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PC-Release"
# PROP Intermediate_Dir "PC-Release"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(X)\x_files" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I "$(X)\x_files" /I "\." /I "$(X)\..\Support" /D "_LIB" /D "$(USERNAME)" /D "WIN32" /D "NDEBUG" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Parsing - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PC-Debug"
# PROP BASE Intermediate_Dir "PC-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PC-Debug"
# PROP Intermediate_Dir "PC-Debug"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(X)\x_files" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "X_DEBUG" /D "X_ASSERT" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "$(X)\x_files" /I "\." /I "$(X)\..\Support" /D "_LIB" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /D "WIN32" /D "_DEBUG" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Parsing - Win32 PS2 DVD Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PS2-DVD-Release"
# PROP BASE Intermediate_Dir "PS2-DVD-Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PS2-DVD-Release"
# PROP Intermediate_Dir "PS2-DVD-Release"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /O2 /I "$(X)\x_files" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "\." /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "Parsing - Win32 PS2 DVD Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PS2-DVD-Debug"
# PROP BASE Intermediate_Dir "PS2-DVD-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2-DVD-Debug"
# PROP Intermediate_Dir "PS2-DVD-Debug"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /Zi /Od /I "$(X)\x_files" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "\." /D "X_DEBUG" /D "X_ASSERT" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "Parsing - Win32 PS2 Client Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PS2-Client-Release"
# PROP BASE Intermediate_Dir "PS2-Client-Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PS2-Client-Release"
# PROP Intermediate_Dir "PS2-Client-Release"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /O2 /I "$(X)\x_files" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "\." /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "Parsing - Win32 PS2 Client Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PS2-Client-Debug"
# PROP BASE Intermediate_Dir "PS2-Client-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2-Client-Debug"
# PROP Intermediate_Dir "PS2-Client-Debug"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /Zi /Od /I "$(X)\x_files" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "\." /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "Parsing - Win32 PS2 DevKit Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PS2-DevKit-Release"
# PROP BASE Intermediate_Dir "PS2-DevKit-Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PS2-DevKit-Release"
# PROP Intermediate_Dir "PS2-DevKit-Release"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /O2 /I "$(X)\x_files" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "\." /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "Parsing - Win32 PS2 DevKit Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PS2-DevKit-Debug"
# PROP BASE Intermediate_Dir "PS2-DevKit-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2-DevKit-Debug"
# PROP Intermediate_Dir "PS2-DevKit-Debug"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /Zi /Od /I "$(X)\x_files" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "\." /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "Parsing - Win32 GCN DevKit Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "GCN-DevKit-Release"
# PROP BASE Intermediate_Dir "GCN-DevKit-Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "GCN-DevKit-Release"
# PROP Intermediate_Dir "GCN-DevKit-Release"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /O2 /D "TARGET_GCN_DEV" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "\." /I "$(X)\..\Support" /D "TARGET_GCN_DEV" /D "VENDOR_SN" /D "$(USERNAME)" /D "HW2" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_GCN_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_GCN_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "Parsing - Win32 GCN DevKit Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "GCN-DevKit-Debug"
# PROP BASE Intermediate_Dir "GCN-DevKit-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "GCN-DevKit-Debug"
# PROP Intermediate_Dir "GCN-DevKit-Debug"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /Zi /Od /D "TARGET_GCN_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "\." /I "$(X)\..\Support" /D "TARGET_GCN_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /D "HW2" /D "_DEBUG" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_GCN_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_GCN_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "Parsing - Win32 GCN DVD Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "GCN-DVD-Release"
# PROP BASE Intermediate_Dir "GCN-DVD-Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "GCN-DVD-Release"
# PROP Intermediate_Dir "GCN-DVD-Release"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /O2 /D "TARGET_GCN_DEV" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "\." /I "$(X)\..\Support" /D "TARGET_GCN_DVD" /D "VENDOR_SN" /D "HW2" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_GCN_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_GCN_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "Parsing - Win32 GCN DVD Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "GCN-DVD-Debug"
# PROP BASE Intermediate_Dir "GCN-DVD-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "GCN-DVD-Debug"
# PROP Intermediate_Dir "GCN-DVD-Debug"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /Zi /Od /D "TARGET_GCN_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "\." /I "$(X)\..\Support" /D "TARGET_GCN_DVD" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "HW2" /D "_DEBUG" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_GCN_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_GCN_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "Parsing - Win32 Xbox Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Parsing___Win32_Xbox_Debug"
# PROP BASE Intermediate_Dir "Parsing___Win32_Xbox_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "$(X)\x_files" /I "\." /I "$(X)\..\Support" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "X_DEBUG" /D "X_ASSERT" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GR /GX /ZI /Od /I "$(X)\x_files" /I "\." /I "$(X)\..\Support" /D "TARGET_XBOX_DEV" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Parsing - Win32 Xbox Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Parsing___Win32_Xbox_Release"
# PROP BASE Intermediate_Dir "Parsing___Win32_Xbox_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /O2 /I "$(X)\x_files" /I "\." /I "$(X)\..\Support" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /O2 /I "$(X)\x_files" /I "\." /I "$(X)\..\Support" /D "TARGET_XBOX_DVD" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "Parsing - Win32 Release"
# Name "Parsing - Win32 Debug"
# Name "Parsing - Win32 PS2 DVD Release"
# Name "Parsing - Win32 PS2 DVD Debug"
# Name "Parsing - Win32 PS2 Client Release"
# Name "Parsing - Win32 PS2 Client Debug"
# Name "Parsing - Win32 PS2 DevKit Release"
# Name "Parsing - Win32 PS2 DevKit Debug"
# Name "Parsing - Win32 GCN DevKit Release"
# Name "Parsing - Win32 GCN DevKit Debug"
# Name "Parsing - Win32 GCN DVD Release"
# Name "Parsing - Win32 GCN DVD Debug"
# Name "Parsing - Win32 Xbox Debug"
# Name "Parsing - Win32 Xbox Release"
# Begin Group "Implementation"

# PROP Default_Filter ""
# Begin Group "external"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\PS2-DevKit-Debug\_SN_in_VC.h"
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_debug.hpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\x_files\x_debug.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_math.hpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\x_files\x_math.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\Implementation\x_math_bb_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\x_files\Implementation\x_math_bb_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\Implementation\x_math_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\x_files\Implementation\x_math_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\Implementation\x_math_m4_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\x_files\Implementation\x_math_m4_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\Implementation\x_math_p_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\x_files\Implementation\x_math_p_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\Implementation\x_math_q_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\x_files\Implementation\x_math_q_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\Implementation\x_math_r3_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\x_files\Implementation\x_math_r3_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\Implementation\x_math_rect_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\x_files\Implementation\x_math_rect_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\Implementation\x_math_v2_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\x_files\Implementation\x_math_v2_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\Implementation\x_math_v3_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\x_files\Implementation\x_math_v3_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\Implementation\x_math_v4_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\x_files\Implementation\x_math_v4_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_memory.hpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\x_files\x_memory.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\Implementation\x_memory_private.hpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\x_files\Implementation\x_memory_private.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_plus.hpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\x_files\x_plus.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_stdio.hpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\x_files\x_stdio.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_target.hpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\x_files\x_target.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_types.hpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\x_files\x_types.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\TextIn.cpp
# End Source File
# Begin Source File

SOURCE=.\TextOut.cpp
# End Source File
# Begin Source File

SOURCE=.\tokenizer.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\TextIn.hpp
# End Source File
# Begin Source File

SOURCE=.\TextOut.hpp
# End Source File
# Begin Source File

SOURCE=.\tokenizer.hpp
# End Source File
# End Target
# End Project
