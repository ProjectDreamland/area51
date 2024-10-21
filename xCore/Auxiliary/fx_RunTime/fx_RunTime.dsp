# Microsoft Developer Studio Project File - Name="fx_RunTime" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104
# TARGTYPE "Xbox Static Library" 0x0b04

CFG=fx_RunTime - Win32 Xbox Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fx_RunTime.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fx_RunTime.mak" CFG="fx_RunTime - Win32 Xbox Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fx_RunTime - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "fx_RunTime - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "fx_RunTime - Xbox Release" (based on "Xbox Static Library")
!MESSAGE "fx_RunTime - Xbox Debug" (based on "Xbox Static Library")
!MESSAGE "fx_RunTime - Win32 PS2 DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "fx_RunTime - Win32 PS2 DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "fx_RunTime - Win32 PS2 Client Release" (based on "Win32 (x86) Static Library")
!MESSAGE "fx_RunTime - Win32 PS2 Client Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "fx_RunTime - Win32 PS2 DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "fx_RunTime - Win32 PS2 DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "fx_RunTime - Win32 GCN DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "fx_RunTime - Win32 GCN DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "fx_RunTime - Win32 GCN DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "fx_RunTime - Win32 GCN DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "fx_RunTime - Win32 Xbox Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "fx_RunTime - Win32 Xbox Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "fx_RunTime"
# PROP Scc_LocalPath "."

!IF  "$(CFG)" == "fx_RunTime - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /D "_LIB" /D "$(USERNAME)" /D "WIN32" /D "NDEBUG" /D "_MBCS" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /D "_LIB" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /D "WIN32" /D "_DEBUG" /D "_MBCS" /YX /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "$(USERNAME)" /D "_LIB" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_XBOX_DEV" /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "$(USERNAME)" /D "_LIB" /YX /FD /G6 /Ztmp /c
RSC=rc.exe
LINK32=link.exe

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "X_DEBUG" /D "X_ASSERT" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_XBOX_DEV" /D "WIN32" /D "_XBOX" /D "_DEBUG" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /D "_LIB" /YX /FD /G6 /Ztmp /c
RSC=rc.exe
LINK32=link.exe

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Release"

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
# ADD CPP /MTd /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "$(USERNAME)" /FD
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Debug"

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
# ADD CPP /MTd /Zi /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /D "X_DEBUG" /D "X_ASSERT" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "$(USERNAME)" /FD
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Release"

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
# ADD CPP /MTd /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "$(USERNAME)" /FD
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Debug"

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
# ADD CPP /MTd /Zi /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /FD
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Release"

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
# ADD CPP /MTd /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "$(USERNAME)" /FD
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Debug"

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
# ADD CPP /MTd /Zi /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /FD
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Release"

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
# ADD CPP /MTd /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_GCN_DEV" /D "HW2" /D "VENDOR_SN" /D "$(USERNAME)" /FD
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_GCN_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_GCN_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Debug"

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
# ADD CPP /MTd /Zi /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_GCN_DEV" /D "X_DEBUG" /D "X_ASSERT" /D "VENDOR_SN" /D "$(USERNAME)" /D "HW2" /FD
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_GCN_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_GCN_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Release"

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
# ADD CPP /MTd /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_GCN_DVD" /D "HW2" /D "VENDOR_SN" /D "$(USERNAME)" /FD
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_GCN_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_GCN_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Debug"

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
# ADD CPP /MTd /Zi /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_GCN_DVD" /D "X_DEBUG" /D "X_ASSERT" /D "HW2" /D "_DEBUG" /D "VENDOR_SN" /D "$(USERNAME)" /FD
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_GCN_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_GCN_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "fx_RunTime___Win32_Xbox_Debug"
# PROP BASE Intermediate_Dir "fx_RunTime___Win32_Xbox_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
LINK32=link.exe
CPP=cl.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_XBOX_DEV" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /FR /YX /FD /GZ /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "fx_RunTime___Win32_Xbox_Release"
# PROP BASE Intermediate_Dir "fx_RunTime___Win32_Xbox_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
LINK32=link.exe
CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "$(USERNAME)" /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_XBOX_DVD" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /YX /FD /c
RSC=rc.exe
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

# Name "fx_RunTime - Win32 Release"
# Name "fx_RunTime - Win32 Debug"
# Name "fx_RunTime - Xbox Release"
# Name "fx_RunTime - Xbox Debug"
# Name "fx_RunTime - Win32 PS2 DVD Release"
# Name "fx_RunTime - Win32 PS2 DVD Debug"
# Name "fx_RunTime - Win32 PS2 Client Release"
# Name "fx_RunTime - Win32 PS2 Client Debug"
# Name "fx_RunTime - Win32 PS2 DevKit Release"
# Name "fx_RunTime - Win32 PS2 DevKit Debug"
# Name "fx_RunTime - Win32 GCN DevKit Release"
# Name "fx_RunTime - Win32 GCN DevKit Debug"
# Name "fx_RunTime - Win32 GCN DVD Release"
# Name "fx_RunTime - Win32 GCN DVD Debug"
# Name "fx_RunTime - Win32 Xbox Debug"
# Name "fx_RunTime - Win32 Xbox Release"
# Begin Group "Implementation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\fx_Ctrl.cpp

!IF  "$(CFG)" == "fx_RunTime - Win32 Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fx_Effect.cpp

!IF  "$(CFG)" == "fx_RunTime - Win32 Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fx_Element.cpp

!IF  "$(CFG)" == "fx_RunTime - Win32 Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fx_Handle.cpp

!IF  "$(CFG)" == "fx_RunTime - Win32 Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fx_LinearKeyCtrl.cpp

!IF  "$(CFG)" == "fx_RunTime - Win32 Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fx_Mgr.cpp

!IF  "$(CFG)" == "fx_RunTime - Win32 Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fx_Mgr_insert.hpp
# End Source File
# Begin Source File

SOURCE=.\fx_Mgr_private.hpp
# End Source File
# Begin Source File

SOURCE=.\fx_SmoothKeyCtrl.cpp

!IF  "$(CFG)" == "fx_RunTime - Win32 Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Release"

!ENDIF 

# End Source File
# End Group
# Begin Group "Elements"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\fx_Cylinder.cpp

!IF  "$(CFG)" == "fx_RunTime - Win32 Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fx_Cylinder.hpp
# End Source File
# Begin Source File

SOURCE=.\fx_Plane.cpp

!IF  "$(CFG)" == "fx_RunTime - Win32 Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fx_Plane.hpp
# End Source File
# Begin Source File

SOURCE=.\fx_ShockWave.cpp

!IF  "$(CFG)" == "fx_RunTime - Win32 Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fx_ShockWave.hpp
# End Source File
# Begin Source File

SOURCE=.\fx_SPEmitter.cpp

!IF  "$(CFG)" == "fx_RunTime - Win32 Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fx_SPEmitter.hpp
# End Source File
# Begin Source File

SOURCE=.\fx_Sphere.cpp

!IF  "$(CFG)" == "fx_RunTime - Win32 Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fx_Sphere.hpp
# End Source File
# Begin Source File

SOURCE=.\fx_Sprite.cpp

!IF  "$(CFG)" == "fx_RunTime - Win32 Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DevKit Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Release"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 GCN DVD Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "fx_RunTime - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fx_Sprite.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\fx_Mgr.hpp
# End Source File
# End Target
# End Project
