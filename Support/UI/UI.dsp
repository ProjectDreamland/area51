# Microsoft Developer Studio Project File - Name="UI" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104
# TARGTYPE "Xbox Static Library" 0x0b04

CFG=UI - Win32 PS2 DevKit Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "UI.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "UI.mak" CFG="UI - Win32 PS2 DevKit Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "UI - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "UI - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "UI - Xbox Release" (based on "Xbox Static Library")
!MESSAGE "UI - Xbox Debug" (based on "Xbox Static Library")
!MESSAGE "UI - Win32 PS2 DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "UI - Win32 PS2 DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "UI - Win32 PS2 Client Release" (based on "Win32 (x86) Static Library")
!MESSAGE "UI - Win32 PS2 Client Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "UI - Win32 PS2 DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "UI - Win32 PS2 DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "UI - Win32 GCN DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "UI - Win32 GCN DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "UI - Win32 GCN DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "UI - Win32 GCN DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "UI"
# PROP Scc_LocalPath "..\..\..\.."

!IF  "$(CFG)" == "UI - Win32 Release"

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
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(X)\x_files" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /MT /W3 /GR /GX /Zi /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\..\Support\Objects" /I "$(X)\Auxiliary" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "EDITOR" /D "UNICODE" /D "$(USERNAME)" /FD /c
# SUBTRACT CPP /YX
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "UI - Win32 Debug"

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
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(X)\x_files" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "X_DEBUG" /D "X_ASSERT" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GR /GX /ZI /Od /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\..\Support\Objects" /I "$(X)\Auxiliary" /D "_DEBUG" /D "X_DEBUG" /D "X_ASSERT" /D "WIN32" /D "_MBCS" /D "_LIB" /D "$(USERNAME)" /Fp"Debug/GameLib.pch" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "UI - Xbox Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Xbox-Release"
# PROP BASE Intermediate_Dir "Xbox-Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Xbox-Release"
# PROP Intermediate_Dir "Xbox-Release"
# PROP Target_Dir ""
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(X)\x_files" /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "$(USERNAME)" /D "_LIB" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\..\Support\Objects" /I "$(X)\Auxiliary" /D "TARGET_XBOX_DEV" /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "$(USERNAME)" /D "_LIB" /YX /FD /G6 /Ztmp /c
RSC=rc.exe
LINK32=link.exe

!ELSEIF  "$(CFG)" == "UI - Xbox Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Xbox-Debug"
# PROP BASE Intermediate_Dir "Xbox-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Xbox-Debug"
# PROP Intermediate_Dir "Xbox-Debug"
# PROP Target_Dir ""
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /I "$(X)\x_files" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "X_DEBUG" /D "X_ASSERT" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /GX /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\..\Support\Objects" /I "$(X)\Auxiliary" /D "TARGET_XBOX_DEV" /D "WIN32" /D "_XBOX" /D "_DEBUG" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /D "_LIB" /YX /FD /G6 /Ztmp /c
# SUBTRACT CPP /Fr
RSC=rc.exe
LINK32=link.exe

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Release"

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
CPP=cl.exe
# ADD BASE CPP /O2 /I "$(X)\x_files" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\..\Support\Objects" /I "$(X)\Auxiliary" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "$(USERNAME)" /FD
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Debug"

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
CPP=cl.exe
# ADD BASE CPP /Zi /Od /I "$(X)\x_files" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\..\Support\Objects" /I "$(X)\Auxiliary" /D "X_DEBUG" /D "X_ASSERT" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "$(USERNAME)" /FD
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Release"

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
CPP=cl.exe
# ADD BASE CPP /O2 /I "$(X)\x_files" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\..\Support\Objects" /I "$(X)\Auxiliary" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "$(USERNAME)" /FD
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Debug"

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
CPP=cl.exe
# ADD BASE CPP /Zi /Od /I "$(X)\x_files" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\..\Support\Objects" /I "$(X)\Auxiliary" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /FD
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Release"

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
CPP=cl.exe
# ADD BASE CPP /O2 /I "$(X)\x_files" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\..\Support\Objects" /I "$(X)\Auxiliary" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "$(USERNAME)" /FD
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Debug"

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
CPP=cl.exe
# ADD BASE CPP /Zi /Od /I "$(X)\x_files" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\..\Support\Objects" /I "$(X)\Auxiliary" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /FD
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Release"

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
CPP=cl.exe
# ADD BASE CPP /O2 /D "TARGET_GCN_DEV" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\..\Support\Objects" /I "$(X)\Auxiliary" /D "TARGET_GCN_DEV" /D "VENDOR_SN" /D "$(USERNAME)" /D "HW2" /FD
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_GCN_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_GCN_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Debug"

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
CPP=cl.exe
# ADD BASE CPP /Zi /Od /D "TARGET_GCN_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\..\Support\Objects" /I "$(X)\Auxiliary" /D "TARGET_GCN_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /D "HW2" /D "_DEBUG" /FD
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_GCN_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_GCN_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Release"

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
CPP=cl.exe
# ADD BASE CPP /O2 /D "TARGET_GCN_DEV" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\..\Support\Objects" /I "$(X)\Auxiliary" /D "TARGET_GCN_DVD" /D "VENDOR_SN" /D "HW2" /D "$(USERNAME)" /FD
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_GCN_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_GCN_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Debug"

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
CPP=cl.exe
# ADD BASE CPP /Zi /Od /D "TARGET_GCN_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\..\Support\Objects" /I "$(X)\Auxiliary" /D "TARGET_GCN_DVD" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "HW2" /D "_DEBUG" /D "$(USERNAME)" /FD
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_GCN_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_GCN_DVD /D:VENDOR_SN

!ENDIF 

# Begin Target

# Name "UI - Win32 Release"
# Name "UI - Win32 Debug"
# Name "UI - Xbox Release"
# Name "UI - Xbox Debug"
# Name "UI - Win32 PS2 DVD Release"
# Name "UI - Win32 PS2 DVD Debug"
# Name "UI - Win32 PS2 Client Release"
# Name "UI - Win32 PS2 Client Debug"
# Name "UI - Win32 PS2 DevKit Release"
# Name "UI - Win32 PS2 DevKit Debug"
# Name "UI - Win32 GCN DevKit Release"
# Name "UI - Win32 GCN DevKit Debug"
# Name "UI - Win32 GCN DVD Release"
# Name "UI - Win32 GCN DVD Debug"
# Begin Group "Implementation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ui_blankbox.cpp

!IF  "$(CFG)" == "UI - Win32 Release"

!ELSEIF  "$(CFG)" == "UI - Win32 Debug"

!ELSEIF  "$(CFG)" == "UI - Xbox Release"

!ELSEIF  "$(CFG)" == "UI - Xbox Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_button.cpp

!IF  "$(CFG)" == "UI - Win32 Release"

!ELSEIF  "$(CFG)" == "UI - Win32 Debug"

!ELSEIF  "$(CFG)" == "UI - Xbox Release"

!ELSEIF  "$(CFG)" == "UI - Xbox Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_check.cpp

!IF  "$(CFG)" == "UI - Win32 Release"

!ELSEIF  "$(CFG)" == "UI - Win32 Debug"

!ELSEIF  "$(CFG)" == "UI - Xbox Release"

!ELSEIF  "$(CFG)" == "UI - Xbox Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_combo.cpp

!IF  "$(CFG)" == "UI - Win32 Release"

!ELSEIF  "$(CFG)" == "UI - Win32 Debug"

!ELSEIF  "$(CFG)" == "UI - Xbox Release"

!ELSEIF  "$(CFG)" == "UI - Xbox Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_control.cpp

!IF  "$(CFG)" == "UI - Win32 Release"

!ELSEIF  "$(CFG)" == "UI - Win32 Debug"

!ELSEIF  "$(CFG)" == "UI - Xbox Release"

!ELSEIF  "$(CFG)" == "UI - Xbox Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_dialog.cpp

!IF  "$(CFG)" == "UI - Win32 Release"

!ELSEIF  "$(CFG)" == "UI - Win32 Debug"

!ELSEIF  "$(CFG)" == "UI - Xbox Release"

!ELSEIF  "$(CFG)" == "UI - Xbox Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_dlg_list.cpp

!IF  "$(CFG)" == "UI - Win32 Release"

!ELSEIF  "$(CFG)" == "UI - Win32 Debug"

!ELSEIF  "$(CFG)" == "UI - Xbox Release"

!ELSEIF  "$(CFG)" == "UI - Xbox Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_dlg_vkeyboard.cpp

!IF  "$(CFG)" == "UI - Win32 Release"

!ELSEIF  "$(CFG)" == "UI - Win32 Debug"

!ELSEIF  "$(CFG)" == "UI - Xbox Release"

!ELSEIF  "$(CFG)" == "UI - Xbox Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_edit.cpp

!IF  "$(CFG)" == "UI - Win32 Release"

!ELSEIF  "$(CFG)" == "UI - Win32 Debug"

!ELSEIF  "$(CFG)" == "UI - Xbox Release"

!ELSEIF  "$(CFG)" == "UI - Xbox Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_font.cpp

!IF  "$(CFG)" == "UI - Win32 Release"

!ELSEIF  "$(CFG)" == "UI - Win32 Debug"

!ELSEIF  "$(CFG)" == "UI - Xbox Release"

!ELSEIF  "$(CFG)" == "UI - Xbox Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_frame.cpp

!IF  "$(CFG)" == "UI - Win32 Release"

!ELSEIF  "$(CFG)" == "UI - Win32 Debug"

!ELSEIF  "$(CFG)" == "UI - Xbox Release"

!ELSEIF  "$(CFG)" == "UI - Xbox Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_joinlist.cpp

!IF  "$(CFG)" == "UI - Win32 Release"

!ELSEIF  "$(CFG)" == "UI - Win32 Debug"

!ELSEIF  "$(CFG)" == "UI - Xbox Release"

!ELSEIF  "$(CFG)" == "UI - Xbox Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_listbox.cpp

!IF  "$(CFG)" == "UI - Win32 Release"

!ELSEIF  "$(CFG)" == "UI - Win32 Debug"

!ELSEIF  "$(CFG)" == "UI - Xbox Release"

!ELSEIF  "$(CFG)" == "UI - Xbox Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_manager.cpp

!IF  "$(CFG)" == "UI - Win32 Release"

!ELSEIF  "$(CFG)" == "UI - Win32 Debug"

!ELSEIF  "$(CFG)" == "UI - Xbox Release"

!ELSEIF  "$(CFG)" == "UI - Xbox Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_playerlist.cpp

!IF  "$(CFG)" == "UI - Win32 Release"

!ELSEIF  "$(CFG)" == "UI - Win32 Debug"

!ELSEIF  "$(CFG)" == "UI - Xbox Release"

!ELSEIF  "$(CFG)" == "UI - Xbox Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_radio.cpp

!IF  "$(CFG)" == "UI - Win32 Release"

!ELSEIF  "$(CFG)" == "UI - Win32 Debug"

!ELSEIF  "$(CFG)" == "UI - Xbox Release"

!ELSEIF  "$(CFG)" == "UI - Xbox Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_slider.cpp

!IF  "$(CFG)" == "UI - Win32 Release"

!ELSEIF  "$(CFG)" == "UI - Win32 Debug"

!ELSEIF  "$(CFG)" == "UI - Xbox Release"

!ELSEIF  "$(CFG)" == "UI - Xbox Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_tabbed_dialog.cpp

!IF  "$(CFG)" == "UI - Win32 Release"

!ELSEIF  "$(CFG)" == "UI - Win32 Debug"

!ELSEIF  "$(CFG)" == "UI - Xbox Release"

!ELSEIF  "$(CFG)" == "UI - Xbox Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_text.cpp

!IF  "$(CFG)" == "UI - Win32 Release"

!ELSEIF  "$(CFG)" == "UI - Win32 Debug"

!ELSEIF  "$(CFG)" == "UI - Xbox Release"

!ELSEIF  "$(CFG)" == "UI - Xbox Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_textbox.cpp

!IF  "$(CFG)" == "UI - Win32 Release"

!ELSEIF  "$(CFG)" == "UI - Win32 Debug"

!ELSEIF  "$(CFG)" == "UI - Xbox Release"

!ELSEIF  "$(CFG)" == "UI - Xbox Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_win.cpp

!IF  "$(CFG)" == "UI - Win32 Release"

!ELSEIF  "$(CFG)" == "UI - Win32 Debug"

!ELSEIF  "$(CFG)" == "UI - Xbox Release"

!ELSEIF  "$(CFG)" == "UI - Xbox Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "UI - Win32 GCN DVD Debug"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\ui_blankbox.hpp
# End Source File
# Begin Source File

SOURCE=.\ui_button.hpp
# End Source File
# Begin Source File

SOURCE=.\ui_check.hpp
# End Source File
# Begin Source File

SOURCE=.\ui_combo.hpp
# End Source File
# Begin Source File

SOURCE=.\ui_control.hpp
# End Source File
# Begin Source File

SOURCE=.\ui_dialog.hpp
# End Source File
# Begin Source File

SOURCE=.\ui_dlg_list.hpp
# End Source File
# Begin Source File

SOURCE=.\ui_dlg_vkeyboard.hpp
# End Source File
# Begin Source File

SOURCE=.\ui_edit.hpp
# End Source File
# Begin Source File

SOURCE=.\ui_font.hpp
# End Source File
# Begin Source File

SOURCE=.\ui_frame.hpp
# End Source File
# Begin Source File

SOURCE=.\ui_joinlist.hpp
# End Source File
# Begin Source File

SOURCE=.\ui_listbox.hpp
# End Source File
# Begin Source File

SOURCE=.\ui_manager.hpp
# End Source File
# Begin Source File

SOURCE=.\ui_playerlist.hpp
# End Source File
# Begin Source File

SOURCE=.\ui_radio.hpp
# End Source File
# Begin Source File

SOURCE=.\ui_slider.hpp
# End Source File
# Begin Source File

SOURCE=.\ui_tabbed_dialog.hpp
# End Source File
# Begin Source File

SOURCE=.\ui_text.hpp
# End Source File
# Begin Source File

SOURCE=.\ui_textbox.hpp
# End Source File
# Begin Source File

SOURCE=.\ui_win.hpp
# End Source File
# End Target
# End Project
