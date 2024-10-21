# Microsoft Developer Studio Project File - Name="MeshUtil" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=MeshUtil - Win32 Xbox Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MeshUtil.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MeshUtil.mak" CFG="MeshUtil - Win32 Xbox Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MeshUtil - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "MeshUtil - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "MeshUtil - Win32 PS2 DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "MeshUtil - Win32 PS2 DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "MeshUtil - Win32 PS2 Client Release" (based on "Win32 (x86) Static Library")
!MESSAGE "MeshUtil - Win32 PS2 Client Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "MeshUtil - Win32 PS2 DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "MeshUtil - Win32 PS2 DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "MeshUtil - Win32 GCN DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "MeshUtil - Win32 GCN DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "MeshUtil - Win32 GCN DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "MeshUtil - Win32 GCN DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "MeshUtil - Win32 Xbox Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "MeshUtil - Win32 Xbox Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "MeshUtil"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MeshUtil - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(X)\MeshUtil" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I "$(X)\Auxiliary\Parsing" /I "$(X)\..\Support" /I "$(X)\MeshUtil" /I "$(X)\x_files" /I "$(X)\Auxiliary\Bitmap" /I "$(X)\Parsing" /I "$(X)" /D "_LIB" /D "$(USERNAME)" /D "WIN32" /D "NDEBUG" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "MeshUtil - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(X)\MeshUtil" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "X_DEBUG" /D "X_ASSERT" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "$(X)\Auxiliary\Parsing" /I "$(X)\..\Support" /I "$(X)\MeshUtil" /I "$(X)\x_files" /I "$(X)\Auxiliary\Bitmap" /I "$(X)\Parsing" /I "$(X)" /D "_LIB" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /D "WIN32" /D "_DEBUG" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "MeshUtil - Win32 PS2 DVD Release"

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
# ADD BASE CPP /O2 /I "$(X)\MeshUtil" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\Auxiliary\Parsing" /I "$(X)\..\Support" /I "$(X)\MeshUtil" /I "$(X)\x_files" /I "$(X)\Auxiliary\Bitmap" /I "$(X)\Parsing" /I "$(X)" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "MeshUtil - Win32 PS2 DVD Debug"

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
# ADD BASE CPP /Zi /Od /I "$(X)\MeshUtil" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\Auxiliary\Parsing" /I "$(X)\..\Support" /I "$(X)\MeshUtil" /I "$(X)\x_files" /I "$(X)\Auxiliary\Bitmap" /I "$(X)\Parsing" /I "$(X)" /D "X_DEBUG" /D "X_ASSERT" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "MeshUtil - Win32 PS2 Client Release"

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
# ADD BASE CPP /O2 /I "$(X)\MeshUtil" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\Auxiliary\Parsing" /I "$(X)\..\Support" /I "$(X)\MeshUtil" /I "$(X)\x_files" /I "$(X)\Auxiliary\Bitmap" /I "$(X)\Parsing" /I "$(X)" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "MeshUtil - Win32 PS2 Client Debug"

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
# ADD BASE CPP /Zi /Od /I "$(X)\MeshUtil" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\Auxiliary\Parsing" /I "$(X)\..\Support" /I "$(X)\MeshUtil" /I "$(X)\x_files" /I "$(X)\Auxiliary\Bitmap" /I "$(X)\Parsing" /I "$(X)" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "MeshUtil - Win32 PS2 DevKit Release"

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
# ADD BASE CPP /O2 /I "$(X)\MeshUtil" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\Auxiliary\Parsing" /I "$(X)\..\Support" /I "$(X)\MeshUtil" /I "$(X)\x_files" /I "$(X)\Auxiliary\Bitmap" /I "$(X)\Parsing" /I "$(X)" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "MeshUtil - Win32 PS2 DevKit Debug"

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
# ADD BASE CPP /Zi /Od /I "$(X)\MeshUtil" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\Auxiliary\Parsing" /I "$(X)\..\Support" /I "$(X)\MeshUtil" /I "$(X)\x_files" /I "$(X)\Auxiliary\Bitmap" /I "$(X)\Parsing" /I "$(X)" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "MeshUtil - Win32 GCN DevKit Release"

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
# ADD CPP /O2 /I "$(X)\Auxiliary\Parsing" /I "$(X)\..\Support" /I "$(X)\MeshUtil" /I "$(X)\x_files" /I "$(X)\Auxiliary\Bitmap" /I "$(X)\Parsing" /I "$(X)" /D "TARGET_GCN_DEV" /D "VENDOR_SN" /D "$(USERNAME)" /D "HW2" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_GCN_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_GCN_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "MeshUtil - Win32 GCN DevKit Debug"

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
# ADD CPP /Zi /Od /I "$(X)\Auxiliary\Parsing" /I "$(X)\..\Support" /I "$(X)\MeshUtil" /I "$(X)\x_files" /I "$(X)\Auxiliary\Bitmap" /I "$(X)\Parsing" /I "$(X)" /D "X_DEBUG" /D "X_ASSERT" /D "_DEBUG" /D "TARGET_GCN_DEV" /D "VENDOR_SN" /D "$(USERNAME)" /D "HW2" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_GCN_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_GCN_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "MeshUtil - Win32 GCN DVD Release"

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
# ADD CPP /O2 /I "$(X)\Auxiliary\Parsing" /I "$(X)\..\Support" /I "$(X)\MeshUtil" /I "$(X)\x_files" /I "$(X)\Auxiliary\Bitmap" /I "$(X)\Parsing" /I "$(X)" /D "TARGET_GCN_DVD" /D "VENDOR_SN" /D "HW2" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_GCN_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_GCN_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "MeshUtil - Win32 GCN DVD Debug"

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
# ADD CPP /Zi /Od /I "$(X)\Auxiliary\Parsing" /I "$(X)\..\Support" /I "$(X)\MeshUtil" /I "$(X)\x_files" /I "$(X)\Auxiliary\Bitmap" /I "$(X)\Parsing" /I "$(X)" /D "TARGET_GCN_DVD" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "HW2" /D "_DEBUG" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_GCN_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_GCN_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "MeshUtil - Win32 Xbox Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "MeshUtil___Win32_Xbox_Debug"
# PROP BASE Intermediate_Dir "MeshUtil___Win32_Xbox_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "$(X)\Auxiliary\Parsing" /I "$(X)\..\Support" /I "$(X)\MeshUtil" /I "$(X)\x_files" /I "$(X)\Auxiliary\Bitmap" /I "$(X)\Parsing" /I "$(X)" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "X_DEBUG" /D "X_ASSERT" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GR /GX /ZI /Od /I "$(X)\Auxiliary\Parsing" /I "$(X)\..\Support" /I "$(X)\MeshUtil" /I "$(X)\x_files" /I "$(X)\Auxiliary\Bitmap" /I "$(X)\Parsing" /I "$(X)" /D "TARGET_XBOX_DEV" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "MeshUtil - Win32 Xbox Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "MeshUtil___Win32_Xbox_Release"
# PROP BASE Intermediate_Dir "MeshUtil___Win32_Xbox_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /O2 /I "$(X)\Auxiliary\Parsing" /I "$(X)\..\Support" /I "$(X)\MeshUtil" /I "$(X)\x_files" /I "$(X)\Auxiliary\Bitmap" /I "$(X)\Parsing" /I "$(X)" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /O2 /I "$(X)\Auxiliary\Parsing" /I "$(X)\..\Support" /I "$(X)\MeshUtil" /I "$(X)\x_files" /I "$(X)\Auxiliary\Bitmap" /I "$(X)\Parsing" /I "$(X)" /D "TARGET_XBOX_DVD" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /YX /FD /c
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

# Name "MeshUtil - Win32 Release"
# Name "MeshUtil - Win32 Debug"
# Name "MeshUtil - Win32 PS2 DVD Release"
# Name "MeshUtil - Win32 PS2 DVD Debug"
# Name "MeshUtil - Win32 PS2 Client Release"
# Name "MeshUtil - Win32 PS2 Client Debug"
# Name "MeshUtil - Win32 PS2 DevKit Release"
# Name "MeshUtil - Win32 PS2 DevKit Debug"
# Name "MeshUtil - Win32 GCN DevKit Release"
# Name "MeshUtil - Win32 GCN DevKit Debug"
# Name "MeshUtil - Win32 GCN DVD Release"
# Name "MeshUtil - Win32 GCN DVD Debug"
# Name "MeshUtil - Win32 Xbox Debug"
# Name "MeshUtil - Win32 Xbox Release"
# Begin Group "Implementation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MatxMisc.cpp
# End Source File
# Begin Source File

SOURCE=.\RawAnim.cpp
# End Source File
# Begin Source File

SOURCE=.\RawMesh.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\MATXMisc.hpp
# End Source File
# Begin Source File

SOURCE=.\RawAnim.hpp
# End Source File
# Begin Source File

SOURCE=.\RawMaterial.cpp
# End Source File
# Begin Source File

SOURCE=.\RawMaterial.hpp
# End Source File
# Begin Source File

SOURCE=.\RawMaterial2.cpp
# End Source File
# Begin Source File

SOURCE=.\RawMaterial2.hpp
# End Source File
# Begin Source File

SOURCE=.\RawMesh.hpp
# End Source File
# Begin Source File

SOURCE=.\RawMesh2.cpp
# End Source File
# Begin Source File

SOURCE=.\RawMesh2.hpp
# End Source File
# End Target
# End Project
