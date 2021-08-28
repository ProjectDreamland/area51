# Microsoft Developer Studio Project File - Name="aux_Bitmap" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=aux_Bitmap - Win32 Xbox Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "aux_Bitmap.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "aux_Bitmap.mak" CFG="aux_Bitmap - Win32 Xbox Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "aux_Bitmap - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "aux_Bitmap - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "aux_Bitmap - Win32 PS2 DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "aux_Bitmap - Win32 PS2 DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "aux_Bitmap - Win32 PS2 Client Release" (based on "Win32 (x86) Static Library")
!MESSAGE "aux_Bitmap - Win32 PS2 Client Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "aux_Bitmap - Win32 PS2 DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "aux_Bitmap - Win32 PS2 DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "aux_Bitmap - Win32 Xbox Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "aux_Bitmap - Win32 Xbox Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "aux_Bitmap"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I "$(X)\x_files" /I "PNG" /I "PNG\ZLib" /I "$(X)\x_files\3rdParty\DXTLibrary" /D "_LIB" /D "$(USERNAME)" /D "WIN32" /D "NDEBUG" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "X_DEBUG" /D "X_ASSERT" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(X)\x_files" /I "PNG" /I "PNG\ZLib" /D "_LIB" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /D "WIN32" /D "_DEBUG" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

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
# ADD BASE CPP /O2 /D "TARGET_PS2_DVD" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "PNG" /I "PNG\ZLib" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

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
# ADD BASE CPP /Zi /Od /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "PNG" /I "PNG\ZLib" /D "X_DEBUG" /D "X_ASSERT" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

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
# ADD BASE CPP /O2 /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /FD
# ADD CPP /Od /I "$(X)\x_files" /I "PNG" /I "PNG\ZLib" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

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
# ADD BASE CPP /Zi /Od /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "PNG" /I "PNG\ZLib" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

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
# ADD BASE CPP /O2 /D "TARGET_PS2_DEV" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "PNG" /I "PNG\ZLib" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

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
# ADD BASE CPP /Zi /Od /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "PNG" /I "PNG\ZLib" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "aux_Bitmap___Win32_Xbox_Debug"
# PROP BASE Intermediate_Dir "aux_Bitmap___Win32_Xbox_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(X)\x_files" /I "PNG" /I "PNG\ZLib" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "X_DEBUG" /D "X_ASSERT" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(X)\x_files" /I "PNG" /I "PNG\ZLib" /D "TARGET_XBOX_DEV" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "aux_Bitmap___Win32_Xbox_Release"
# PROP BASE Intermediate_Dir "aux_Bitmap___Win32_Xbox_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /O2 /I "$(X)\x_files" /I "PNG" /I "PNG\ZLib" /I "$(X)\x_files\3rdParty\DXTLibrary" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /O2 /I "$(X)\x_files" /I "PNG" /I "PNG\ZLib" /I "$(X)\x_files\3rdParty\DXTLibrary" /D "TARGET_XBOX_DVD" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /YX /FD /c
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

# Name "aux_Bitmap - Win32 Release"
# Name "aux_Bitmap - Win32 Debug"
# Name "aux_Bitmap - Win32 PS2 DVD Release"
# Name "aux_Bitmap - Win32 PS2 DVD Debug"
# Name "aux_Bitmap - Win32 PS2 Client Release"
# Name "aux_Bitmap - Win32 PS2 Client Debug"
# Name "aux_Bitmap - Win32 PS2 DevKit Release"
# Name "aux_Bitmap - Win32 PS2 DevKit Debug"
# Name "aux_Bitmap - Win32 Xbox Debug"
# Name "aux_Bitmap - Win32 Xbox Release"
# Begin Group "Implementation"

# PROP Default_Filter ""
# Begin Group "External"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\x_files\x_bitmap.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\Implementation\x_bitmap_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_color.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\Implementation\x_color_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_debug.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_math.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\Implementation\x_math_bb_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\Implementation\x_math_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\Implementation\x_math_m4_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\Implementation\x_math_p_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\Implementation\x_math_q_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\Implementation\x_math_r3_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\Implementation\x_math_v2_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\Implementation\x_math_v3_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\Implementation\x_math_v4_inline.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_memory.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\Implementation\x_memory_private.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_plus.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_stdio.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_target.hpp
# End Source File
# Begin Source File

SOURCE=..\..\x_files\x_types.hpp
# End Source File
# End Group
# Begin Group "DXT"

# PROP Default_Filter ""
# Begin Group "Codebooks"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\CodeBook4MMX.cpp

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\CodeBook4MMX.h

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\fCodeBook.cpp

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\fCodeBook.h

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# End Group
# Begin Group "Boneyard"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\CodeBook.cpp

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\CodeBook.h

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\LoadFromBMP.cpp

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\LoadFromTGA.cpp

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\LoadImageFrom.cpp

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\LoadImageFrom.h

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\Usage.cpp

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\ccDoubleHeap.cpp

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\ccDoubleHeap.h

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\ccHeap.cpp

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\ccHeap.h

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\Cclist.cpp

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\Cclist.h

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\cfVector.cpp

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\cfVector.h

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\DXTCGen.cpp

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\DXTCGen.h

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\Image.cpp

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\Image.h

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\ImageDXTC.cpp

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\ImageDXTC.h

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\Lloyd.cpp

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\Lloyd.h

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\MedianCut.cpp

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\MedianCut.h

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\Table.cpp

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdParty\DXTLibrary\Table.h

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\aux_Bitmap.cpp

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DefaultBMP.cpp

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Load_BMP.cpp

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Load_PSD.cpp
# End Source File
# Begin Source File

SOURCE=.\Load_TGA.cpp

!IF  "$(CFG)" == "aux_Bitmap - Win32 Release"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DVD Debug"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 Client Debug"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Debug"

!ELSEIF  "$(CFG)" == "aux_Bitmap - Win32 Xbox Release"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\aux_Bitmap.hpp
# End Source File
# End Target
# End Project
