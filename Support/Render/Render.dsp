# Microsoft Developer Studio Project File - Name="Render" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Render - Win32 Xbox Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Render.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Render.mak" CFG="Render - Win32 Xbox Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Render - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Render - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Render - Win32 PS2 DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Render - Win32 PS2 DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Render - Win32 PS2 Client Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Render - Win32 PS2 Client Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Render - Win32 PS2 DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Render - Win32 PS2 DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Render - Win32 Xbox Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Render - Win32 Xbox Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Render"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Render - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(X)\x_files" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /D "_LIB" /D "$(USERNAME)" /D "WIN32" /D "NDEBUG" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(X)\x_files" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "X_DEBUG" /D "X_ASSERT" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /D "_LIB" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /D "WIN32" /D "_DEBUG" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

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
# ADD CPP /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

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
# ADD CPP /Zi /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /D "X_DEBUG" /D "X_ASSERT" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

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
# ADD CPP /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

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
# ADD CPP /Zi /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

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
# ADD CPP /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

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
# ADD CPP /Zi /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Render___Win32_Xbox_Debug"
# PROP BASE Intermediate_Dir "Render___Win32_Xbox_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /D "TARGET_XBOX_DEV" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Render___Win32_Xbox_Release"
# PROP BASE Intermediate_Dir "Render___Win32_Xbox_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "$(USERNAME)" /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /D "TARGET_XBOX_DVD" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /YX /FD /c
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

# Name "Render - Win32 Release"
# Name "Render - Win32 Debug"
# Name "Render - Win32 PS2 DVD Release"
# Name "Render - Win32 PS2 DVD Debug"
# Name "Render - Win32 PS2 Client Release"
# Name "Render - Win32 PS2 Client Debug"
# Name "Render - Win32 PS2 DevKit Release"
# Name "Render - Win32 PS2 DevKit Debug"
# Name "Render - Win32 Xbox Debug"
# Name "Render - Win32 Xbox Release"
# Begin Group "PS2"

# PROP Default_Filter "cpp;hpp;vu"
# Begin Group "vu0"

# PROP Default_Filter "cpp;hpp"
# Begin Source File

SOURCE=.\vu0\mcode\include0.vu

!IF  "$(CFG)" == "Render - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vu0\mcode\skinning.vu

!IF  "$(CFG)" == "Render - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vu0\vu0.cpp

!IF  "$(CFG)" == "Render - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vu0\vu0.hpp

!IF  "$(CFG)" == "Render - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "vu1"

# PROP Default_Filter "cpp;hpp"
# Begin Source File

SOURCE=.\vu1\mcode\clipper.vu

!IF  "$(CFG)" == "Render - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vu1\draw.cpp

!IF  "$(CFG)" == "Render - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vu1\mcode\include.vu

!IF  "$(CFG)" == "Render - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vu1\mcode\lighting.vu

!IF  "$(CFG)" == "Render - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vu1\mcode\passes.vu

!IF  "$(CFG)" == "Render - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vu1\mcode\renderer.vu

!IF  "$(CFG)" == "Render - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vu1\mcode\shadow.vu

!IF  "$(CFG)" == "Render - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vu1\mcode\shadows.vu

!IF  "$(CFG)" == "Render - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vu1\mcode\skinning.vu

!IF  "$(CFG)" == "Render - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vu1\mcode\transform.vu

!IF  "$(CFG)" == "Render - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vu1\vu1.cpp

!IF  "$(CFG)" == "Render - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vu1\vu1.hpp

!IF  "$(CFG)" == "Render - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vu1\vu1_dlist.cpp

!IF  "$(CFG)" == "Render - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\ps2_post.cpp

!IF  "$(CFG)" == "Render - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ps2_Render.cpp

!IF  "$(CFG)" == "Render - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ps2_skin.cpp

!IF  "$(CFG)" == "Render - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ps2_skin.hpp

!IF  "$(CFG)" == "Render - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "PC"

# PROP Default_Filter "cpp;hpp"
# Begin Group "Shaders"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Shaders\Make.bat
# End Source File
# Begin Source File

SOURCE=.\Shaders\SkinShader.h
# End Source File
# Begin Source File

SOURCE=.\Shaders\SkinShader.vsa
# End Source File
# Begin Source File

SOURCE=.\Shaders\SkinShader.vsa.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\pc_Render.cpp

!IF  "$(CFG)" == "Render - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SoftVertexMgr.cpp

!IF  "$(CFG)" == "Render - Win32 Release"

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SoftVertexMgr.hpp

!IF  "$(CFG)" == "Render - Win32 Release"

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\VertexMgr.cpp

!IF  "$(CFG)" == "Render - Win32 Release"

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\VertexMgr.hpp

!IF  "$(CFG)" == "Render - Win32 Release"

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\CollisionVolume.cpp
# End Source File
# Begin Source File

SOURCE=.\CollisionVolume.hpp
# End Source File
# Begin Source File

SOURCE=.\geom.cpp
# End Source File
# Begin Source File

SOURCE=.\geom.hpp
# End Source File
# Begin Source File

SOURCE=.\LightMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\LightMgr.hpp
# End Source File
# Begin Source File

SOURCE=.\Material.cpp
# End Source File
# Begin Source File

SOURCE=.\Material.hpp
# End Source File
# Begin Source File

SOURCE=.\Material_Prefs.hpp

!IF  "$(CFG)" == "Render - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Render - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MaterialArray.cpp
# End Source File
# Begin Source File

SOURCE=.\MaterialArray.hpp
# End Source File
# Begin Source File

SOURCE=.\platform_Render.hpp
# End Source File
# Begin Source File

SOURCE=.\ProjTextureMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\ProjTextureMgr.hpp
# End Source File
# Begin Source File

SOURCE=.\Render.cpp
# End Source File
# Begin Source File

SOURCE=.\Render.hpp
# End Source File
# Begin Source File

SOURCE=.\RigidGeom.cpp
# End Source File
# Begin Source File

SOURCE=.\RigidGeom.hpp
# End Source File
# Begin Source File

SOURCE=.\SkinGeom.cpp
# End Source File
# Begin Source File

SOURCE=.\SkinGeom.hpp
# End Source File
# Begin Source File

SOURCE=.\TexAnimArray.cpp
# End Source File
# Begin Source File

SOURCE=.\TexAnimArray.hpp
# End Source File
# Begin Source File

SOURCE=.\Texture.cpp
# End Source File
# Begin Source File

SOURCE=.\Texture.hpp
# End Source File
# End Target
# End Project
