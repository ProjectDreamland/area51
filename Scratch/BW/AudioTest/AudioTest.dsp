# Microsoft Developer Studio Project File - Name="*AudioTest*" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101
# TARGTYPE "Xbox Application" 0x0b01

CFG=*AudioTest* - Win32 PS2 DevKit Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "AudioTest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AudioTest.mak" CFG="*AudioTest* - Win32 PS2 DevKit Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "*AudioTest* - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "*AudioTest* - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "*AudioTest* - Win32 PS2 DVD Release" (based on "Win32 (x86) Application")
!MESSAGE "*AudioTest* - Win32 PS2 DVD Debug" (based on "Win32 (x86) Application")
!MESSAGE "*AudioTest* - Win32 PS2 Client Release" (based on "Win32 (x86) Application")
!MESSAGE "*AudioTest* - Win32 PS2 Client Debug" (based on "Win32 (x86) Application")
!MESSAGE "*AudioTest* - Win32 PS2 DevKit Release" (based on "Win32 (x86) Application")
!MESSAGE "*AudioTest* - Win32 PS2 DevKit Debug" (based on "Win32 (x86) Application")
!MESSAGE "*AudioTest* - Xbox Release" (based on "Xbox Application")
!MESSAGE "*AudioTest* - Xbox Debug" (based on "Xbox Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "AudioTest"
# PROP Scc_LocalPath "."

!IF  "$(CFG)" == "*AudioTest* - Win32 Release"

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
XBE=imagebld.exe
XBCP=xbecopy.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
MTL=midl.exe
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 Debug"

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
XBE=imagebld.exe
XBCP=xbecopy.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(X)\x_files" /I "$(X)\Entropy" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "X_DEBUG" /D "X_ASSERT" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(X)\x_files" /I "$(X)\Entropy" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "X_DEBUG" /D "X_ASSERT" /YX /FD /GZ /c
MTL=midl.exe
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 DVD Release"

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
XBE=imagebld.exe
XBCP=xbecopy.exe
CPP=cl.exe
# ADD BASE CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /FD
MTL=midl.exe
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86 /out:"PS2-DVD-Release/AudioTest.elf" /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LINK32 /machine:IX86 /out:"PS2-DVD-Release/AudioTest.elf" /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 DVD Debug"

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
XBE=imagebld.exe
XBCP=xbecopy.exe
CPP=cl.exe
# ADD BASE CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
MTL=midl.exe
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /debug /machine:IX86 /out:"PS2-DVD-Debug/AudioTest.elf" /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LINK32 /debug /machine:IX86 /out:"PS2-DVD-Debug/AudioTest.elf" /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 Client Release"

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
XBE=imagebld.exe
XBCP=xbecopy.exe
CPP=cl.exe
# ADD BASE CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /FD
MTL=midl.exe
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86 /out:"PS2-Client-Release/AudioTest.elf" /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LINK32 /machine:IX86 /out:"PS2-Client-Release/AudioTest.elf" /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 Client Debug"

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
XBE=imagebld.exe
XBCP=xbecopy.exe
CPP=cl.exe
# ADD BASE CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
MTL=midl.exe
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /debug /machine:IX86 /out:"PS2-Client-Debug/AudioTest.elf" /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LINK32 /debug /machine:IX86 /out:"PS2-Client-Debug/AudioTest.elf" /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 DevKit Release"

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
XBE=imagebld.exe
XBCP=xbecopy.exe
CPP=cl.exe
# ADD BASE CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /FD
MTL=midl.exe
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86 /out:"PS2-DevKit-Release/AudioTest.elf" /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LINK32 /machine:IX86 /out:"PS2-DevKit-Release/AudioTest.elf" /D:TARGET_PS2_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 DevKit Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Xbox-DevKit-Debug"
# PROP BASE Intermediate_Dir "Xbox-DevKit-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2-DevKit-Debug"
# PROP Intermediate_Dir "PS2-DevKit-Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
XBE=imagebld.exe
# ADD BASE XBE /nologo /stack:0x10000 /nologo /stack:0x10000 /debug
# ADD XBE /nologo /stack:0x10000 /nologo /stack:0x10000 /debug
XBCP=xbecopy.exe
# ADD BASE XBCP /NOLOGO /NOLOGO
# ADD XBCP /NOLOGO /NOLOGO
CPP=cl.exe
# ADD BASE CPP /nologo /G6 /W3 /Gm /GX /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "_XBOX" /D "NDEBUG" /D "_DEBUG" /YX /FD /Zvc6 /Zvc6 /c
# ADD CPP /nologo /G6 /W3 /Gm /GX /Zi /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "_XBOX" /D "NDEBUG" /D "_DEBUG" /YX /FD /c
MTL=midl.exe
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib xapilibd.lib d3d8d.lib d3dx8d.lib xgraphicsd.lib dsoundd.lib dmusicd.lib xnetd.lib xboxkrnl.lib /nologo /incremental:no /debug /machine:I386 /out:"PS2-DevKit-Debug/AudioTest.elf" /D:TARGET_PS2_DEV /D:VENDOR_SN /subsystem:xbox /fixed:no /debugtype:vc6 /OPT:REF /subsystem:xbox /fixed:no /debugtype:vc6
# ADD LINK32 sntty.lib /nologo /incremental:no /debug /machine:I386 /out:"PS2-DevKit-Debug/AudioTest.elf" /D:TARGET_PS2_DEV /D:VENDOR_SN
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "*AudioTest* - Xbox Release"

RSC=rc.exe
CPP=cl.exe
BSC32=bscmake.exe
LINK32=link.exe
XBE=imagebld.exe
XBCP=xbecopy.exe

!ELSEIF  "$(CFG)" == "*AudioTest* - Xbox Debug"

RSC=rc.exe
CPP=cl.exe
BSC32=bscmake.exe
LINK32=link.exe
XBE=imagebld.exe
XBCP=xbecopy.exe

!ENDIF 

# Begin Target

# Name "*AudioTest* - Win32 Release"
# Name "*AudioTest* - Win32 Debug"
# Name "*AudioTest* - Win32 PS2 DVD Release"
# Name "*AudioTest* - Win32 PS2 DVD Debug"
# Name "*AudioTest* - Win32 PS2 Client Release"
# Name "*AudioTest* - Win32 PS2 Client Debug"
# Name "*AudioTest* - Win32 PS2 DevKit Release"
# Name "*AudioTest* - Win32 PS2 DevKit Debug"
# Name "*AudioTest* - Xbox Release"
# Name "*AudioTest* - Xbox Debug"
# Begin Source File

SOURCE=.\crt0.s

!IF  "$(CFG)" == "*AudioTest* - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "*AudioTest* - Xbox Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "*AudioTest* - Xbox Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\main.cpp

!IF  "$(CFG)" == "*AudioTest* - Win32 Release"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 Debug"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "*AudioTest* - Xbox Release"

NODEP_CPP_MAIN_=\
	".\AudioMgr\AudioMgr.hpp"\
	".\entropy.hpp"\
	".\Entropy\Audio\io_mgr.hpp"\
	".\Gamelib\Link.hpp"\
	

!ELSEIF  "$(CFG)" == "*AudioTest* - Xbox Debug"

NODEP_CPP_MAIN_=\
	".\AudioMgr\AudioMgr.hpp"\
	".\entropy.hpp"\
	".\Entropy\Audio\io_mgr.hpp"\
	".\Gamelib\Link.hpp"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PS2.lk
# PROP Exclude_From_Scan -1
# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1
# End Source File
# End Target
# End Project
