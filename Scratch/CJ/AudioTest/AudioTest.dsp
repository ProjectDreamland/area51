# Microsoft Developer Studio Project File - Name="*AudioTest*" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
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
!MESSAGE "*AudioTest* - Xbox Release" (based on "Xbox Application")
!MESSAGE "*AudioTest* - Xbox Debug" (based on "Xbox Application")
!MESSAGE "*AudioTest* - Win32 PS2 DVD Release" (based on "Win32 (x86) Application")
!MESSAGE "*AudioTest* - Win32 PS2 DVD Debug" (based on "Win32 (x86) Application")
!MESSAGE "*AudioTest* - Win32 PS2 Client Release" (based on "Win32 (x86) Application")
!MESSAGE "*AudioTest* - Win32 PS2 Client Debug" (based on "Win32 (x86) Application")
!MESSAGE "*AudioTest* - Win32 PS2 DevKit Release" (based on "Win32 (x86) Application")
!MESSAGE "*AudioTest* - Win32 PS2 DevKit Debug" (based on "Win32 (x86) Application")
!MESSAGE "*AudioTest* - Win32 GCN DevKit Release" (based on "Win32 (x86) Application")
!MESSAGE "*AudioTest* - Win32 GCN DevKit Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath "..\.."

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
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\Entropy\IOManager" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
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
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(X)\x_files" /I "$(X)\Entropy" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "X_DEBUG" /D "X_ASSERT" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\Entropy\IOManager" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "X_DEBUG" /D "X_ASSERT" /YX /FD /GZ /c
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

!ELSEIF  "$(CFG)" == "*AudioTest* - Xbox Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Xbox-Release"
# PROP BASE Intermediate_Dir "Xbox-Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Xbox-Release"
# PROP Intermediate_Dir "Xbox-Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
XBCP=xbecopy.exe
# ADD BASE XBCP /NOLOGO
# ADD XBCP /NOLOGO
XBE=imagebld.exe
# ADD BASE XBE /nologo /stack:0x10000
# ADD XBE /nologo /testname:"AudioTest" /stack:0x10000 /titleimage:"titleimage.xbx"
# SUBTRACT XBE /limitmem
LINK32=link.exe
# ADD BASE LINK32 xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib /nologo /machine:I386 /subsystem:xbox /fixed:no /TMP /OPT:REF /subsystem:xbox /fixed:no /TMP /OPT:REF /subsystem:xbox /fixed:no /TMP /OPT:REF /subsystem:xbox /fixed:no /TMP /OPT:REF
# ADD LINK32 xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib /nologo /debug /machine:I386 /subsystem:xbox /fixed:no /TMP /OPT:REF /subsystem:xbox /fixed:no /TMP /OPT:REF /subsystem:xbox /fixed:no /TMP /OPT:REF /subsystem:xbox /fixed:no /TMP /OPT:REF
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_XBOX" /D "WIN32" /D "NDEBUG" /D "_XBOX" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /I "." /I "$(X)\Auxiliary" /I "$(X)\Auxiliary\Bitmap" /I "$(x)\Auxiliary\Parsing" /I "$(X)\..\Support" /I "$(X)\..\Support\Objects" /I "$(X)\3rdparty\xbox\other\include" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\Entropy\IOManager" /D "TARGET_XBOX_DEV" /D "WIN32" /D "NDEBUG" /D "_XBOX" /D "X_ASSERT" /D "$(USERNAME)" /YX /FD /I /D /G6 /Ztmp /c
RSC=rc.exe

!ELSEIF  "$(CFG)" == "*AudioTest* - Xbox Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Xbox-Debug"
# PROP BASE Intermediate_Dir "Xbox-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Xbox-Debug"
# PROP Intermediate_Dir "Xbox-Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
XBCP=xbecopy.exe
# ADD BASE XBCP /NOLOGO
# ADD XBCP /NOLOGO
XBE=imagebld.exe
# ADD BASE XBE /nologo /stack:0x10000 /debug
# ADD XBE /nologo /testname:"AudioTest" /stack:0x40000 /debug /titleimage:"titleimage.xbx"
# SUBTRACT XBE /limitmem
LINK32=link.exe
# ADD BASE LINK32 xapilibd.lib d3d8d.lib d3dx8d.lib xgraphicsd.lib dsoundd.lib dmusicd.lib xnetd.lib xboxkrnl.lib xapilibd.lib d3d8d.lib d3dx8d.lib xgraphicsd.lib dsoundd.lib dmusicd.lib xnetd.lib xboxkrnl.lib /nologo /incremental:no /debug /machine:I386 /subsystem:xbox /fixed:no /TMP /subsystem:xbox /fixed:no /TMP
# ADD LINK32 xapilibd.lib d3d8d.lib d3dx8d.lib xgraphicsd.lib dsound.lib dmusicd.lib xnetd.lib xboxkrnl.lib xapilibd.lib d3d8d.lib d3dx8d.lib xgraphicsd.lib dsoundd.lib dmusicd.lib xnetd.lib xboxkrnl.lib /nologo /incremental:no /debug /machine:I386 /subsystem:xbox /fixed:no /subsystem:xbox /fixed:no /TMP
# SUBTRACT LINK32 /pdb:none
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /D "WIN32" /D "_DEBUG" /D "_XBOX" /D "X_DEBUG" /D "X_ASSERT" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /GX /Zi /Od /I "." /I "$(X)\Auxiliary" /I "$(X)\Auxiliary\Bitmap" /I "$(x)\Auxiliary\Parsing" /I "$(X)\..\Support" /I "$(X)\..\Support\Objects" /I "$(X)\3rdparty\xbox\other\include" /I "C:\Projects\MeridianBranch\xCore\entropy\IOManager" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\Entropy\IOManager" /D "TARGET_XBOX_DEV" /D "WIN32" /D "_DEBUG" /D "_XBOX" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /YX /FD /I /G6 /Ztmp /c
RSC=rc.exe

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
CPP=cl.exe
# ADD BASE CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\Entropy\IOManager" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /FD
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
CPP=cl.exe
# ADD BASE CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\Entropy\IOManager" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
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
CPP=cl.exe
# ADD BASE CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\Entropy\IOManager" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /FD
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
CPP=cl.exe
# ADD BASE CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\Entropy\IOManager" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
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
CPP=cl.exe
# ADD BASE CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\Entropy\IOManager" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /FD
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
# PROP BASE Output_Dir "PS2-DevKit-Debug"
# PROP BASE Intermediate_Dir "PS2-DevKit-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2-DevKit-Debug"
# PROP Intermediate_Dir "PS2-DevKit-Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\Entropy\IOManager" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
MTL=midl.exe
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /debug /machine:IX86 /out:"PS2-DevKit-Debug/AudioTest.elf" /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LINK32 /debug /machine:IX86 /out:"PS2-DevKit-Debug/AudioTest.elf" /D:TARGET_PS2_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 GCN DevKit Release"

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
CPP=cl.exe
# ADD BASE CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_GCN_DEV" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\Entropy\IOManager" /D "TARGET_GCN_DEV" /D "VENDOR_SN" /D "HW2" /FD
MTL=midl.exe
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86 /out:"GCN-DevKit-Release/AudioTest.elf" /D:TARGET_GCN_DEV /D:VENDOR_SN
# ADD LINK32 /machine:IX86 /out:"GCN-DevKit-Release/AudioTest.elf" /D:TARGET_GCN_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 GCN DevKit Debug"

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
CPP=cl.exe
# ADD BASE CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /D "TARGET_GCN_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\Entropy\IOManager" /D "TARGET_GCN_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "HW2" /D "_DEBUG" /FD
MTL=midl.exe
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /debug /machine:IX86 /out:"GCN-DevKit-Debug/AudioTest.elf" /D:TARGET_GCN_DEV /D:VENDOR_SN
# ADD LINK32 /debug /machine:IX86 /out:"GCN-DevKit-Debug/AudioTest.elf" /D:TARGET_GCN_DEV /D:VENDOR_SN

!ENDIF 

# Begin Target

# Name "*AudioTest* - Win32 Release"
# Name "*AudioTest* - Win32 Debug"
# Name "*AudioTest* - Xbox Release"
# Name "*AudioTest* - Xbox Debug"
# Name "*AudioTest* - Win32 PS2 DVD Release"
# Name "*AudioTest* - Win32 PS2 DVD Debug"
# Name "*AudioTest* - Win32 PS2 Client Release"
# Name "*AudioTest* - Win32 PS2 Client Debug"
# Name "*AudioTest* - Win32 PS2 DevKit Release"
# Name "*AudioTest* - Win32 PS2 DevKit Debug"
# Name "*AudioTest* - Win32 GCN DevKit Release"
# Name "*AudioTest* - Win32 GCN DevKit Debug"
# Begin Source File

SOURCE=.\AudioTest.cpp

!IF  "$(CFG)" == "*AudioTest* - Win32 Release"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 Debug"

!ELSEIF  "$(CFG)" == "*AudioTest* - Xbox Release"

!ELSEIF  "$(CFG)" == "*AudioTest* - Xbox Debug"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 GCN DevKit Release"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 GCN DevKit Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crt0.s

!IF  "$(CFG)" == "*AudioTest* - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "*AudioTest* - Xbox Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "*AudioTest* - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 PS2 DevKit Debug"

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 GCN DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "*AudioTest* - Win32 GCN DevKit Debug"

# PROP Exclude_From_Build 1

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
