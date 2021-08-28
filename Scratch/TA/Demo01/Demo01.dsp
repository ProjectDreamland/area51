# Microsoft Developer Studio Project File - Name="*Demo01*" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=*Demo01* - Win32 PS2 DevKit Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Demo01.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Demo01.mak" CFG="*Demo01* - Win32 PS2 DevKit Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "*Demo01* - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "*Demo01* - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "*Demo01* - Win32 PS2 DVD Release" (based on "Win32 (x86) Application")
!MESSAGE "*Demo01* - Win32 PS2 DVD Debug" (based on "Win32 (x86) Application")
!MESSAGE "*Demo01* - Win32 PS2 Client Release" (based on "Win32 (x86) Application")
!MESSAGE "*Demo01* - Win32 PS2 Client Debug" (based on "Win32 (x86) Application")
!MESSAGE "*Demo01* - Win32 PS2 DevKit Release" (based on "Win32 (x86) Application")
!MESSAGE "*Demo01* - Win32 PS2 DevKit Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Demo01"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "*Demo01* - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(X)\x_files" /I "$(X)\Entropy" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "X_DEBUG" /D "X_ASSERT" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(X)\x_files" /I "$(X)\Entropy" /I ".\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "X_DEBUG" /D "X_ASSERT" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DVD Release"

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
# ADD CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86 /out:"PS2-DVD-Release/Demo01.elf" /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LINK32 /machine:IX86 /out:"PS2-DVD-Release/Demo01.elf" /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DVD Debug"

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
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /debug /machine:IX86 /out:"PS2-DVD-Debug/Demo01.elf" /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LINK32 /debug /machine:IX86 /out:"PS2-DVD-Debug/Demo01.elf" /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 Client Release"

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
# ADD CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86 /out:"PS2-Client-Release/Demo01.elf" /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LINK32 /machine:IX86 /out:"PS2-Client-Release/Demo01.elf" /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 Client Debug"

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
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /debug /machine:IX86 /out:"PS2-Client-Debug/Demo01.elf" /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LINK32 /debug /machine:IX86 /out:"PS2-Client-Debug/Demo01.elf" /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DevKit Release"

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
# ADD CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86 /out:"PS2-DevKit-Release/Demo01.elf" /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LINK32 /machine:IX86 /out:"PS2-DevKit-Release/Demo01.elf" /D:TARGET_PS2_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DevKit Debug"

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
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /debug /machine:IX86 /out:"PS2-DevKit-Debug/Demo01.elf" /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LINK32 /debug /machine:IX86 /out:"PS2-DevKit-Debug/Demo01.elf" /D:TARGET_PS2_DEV /D:VENDOR_SN

!ENDIF 

# Begin Target

# Name "*Demo01* - Win32 Release"
# Name "*Demo01* - Win32 Debug"
# Name "*Demo01* - Win32 PS2 DVD Release"
# Name "*Demo01* - Win32 PS2 DVD Debug"
# Name "*Demo01* - Win32 PS2 Client Release"
# Name "*Demo01* - Win32 PS2 Client Debug"
# Name "*Demo01* - Win32 PS2 DevKit Release"
# Name "*Demo01* - Win32 PS2 DevKit Debug"
# Begin Group "NotCompile"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\col_volumen.cpp

!IF  "$(CFG)" == "*Demo01* - Win32 Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DevKit Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\col_volumen.hpp

!IF  "$(CFG)" == "*Demo01* - Win32 Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DevKit Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\coll.cpp

!IF  "$(CFG)" == "*Demo01* - Win32 Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DevKit Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\coll.hpp

!IF  "$(CFG)" == "*Demo01* - Win32 Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DevKit Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CollisionMgr.cpp

!IF  "$(CFG)" == "*Demo01* - Win32 Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DevKit Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CollisionMgr.hpp

!IF  "$(CFG)" == "*Demo01* - Win32 Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DevKit Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CollisionPrimatives.cpp

!IF  "$(CFG)" == "*Demo01* - Win32 Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DevKit Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CollisionPrimatives.hpp

!IF  "$(CFG)" == "*Demo01* - Win32 Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DevKit Debug"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\crt0.s

!IF  "$(CFG)" == "*Demo01* - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "*Demo01* - Win32 PS2 DevKit Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\PS2.lk
# PROP Exclude_From_Scan -1
# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Spatial_user.hpp
# End Source File
# Begin Source File

SOURCE=.\SpatialDBase.cpp
# End Source File
# Begin Source File

SOURCE=.\SpatialDBase.hpp
# End Source File
# Begin Source File

SOURCE=.\SpatialDBase_inline.hpp
# End Source File
# End Target
# End Project
