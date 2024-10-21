# Microsoft Developer Studio Project File - Name="NetworkMgr" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=NetworkMgr - Win32 PS2 DevKit Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "NetworkMgr.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "NetworkMgr.mak" CFG="NetworkMgr - Win32 PS2 DevKit Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "NetworkMgr - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "NetworkMgr - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "NetworkMgr - Win32 PS2 DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "NetworkMgr - Win32 PS2 DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "NetworkMgr - Win32 PS2 Client Release" (based on "Win32 (x86) Static Library")
!MESSAGE "NetworkMgr - Win32 PS2 Client Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "NetworkMgr - Win32 PS2 DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "NetworkMgr - Win32 PS2 DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /I "$(X)" /I "$(X)\..\Support" /I "$(X)\Entropy" /I "$(X)\x_files" /I "$(X)\Auxiliary" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "$(USERNAME)" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

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
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(X)" /I "$(X)\..\Support" /I "$(X)\Entropy" /I "$(X)\x_files" /I "$(X)\Auxiliary" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

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
# ADD CPP /O2 /I "$(X)" /I "$(X)\..\Support" /I "$(X)\Entropy" /I "$(X)\x_files" /I "$(X)\Auxiliary" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

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
# ADD CPP /Zi /Od /I "$(X)" /I "$(X)\..\Support" /I "$(X)\Entropy" /I "$(X)\x_files" /I "$(X)\Auxiliary" /D "X_DEBUG" /D "X_ASSERT" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

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
# ADD CPP /O2 /I "$(X)" /I "$(X)\..\Support" /I "$(X)\Entropy" /I "$(X)\x_files" /I "$(X)\Auxiliary" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

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
# ADD CPP /Zi /Od /I "$(X)" /I "$(X)\..\Support" /I "$(X)\Entropy" /I "$(X)\x_files" /I "$(X)\Auxiliary" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

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
# ADD CPP /O2 /I "$(X)" /I "$(X)\..\Support" /I "$(X)\Entropy" /I "$(X)\x_files" /I "$(X)\Auxiliary" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

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
# ADD CPP /Zi /Od /I "$(X)" /I "$(X)\..\Support" /I "$(X)\Entropy" /I "$(X)\x_files" /I "$(X)\Auxiliary" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN

!ENDIF 

# Begin Target

# Name "NetworkMgr - Win32 Release"
# Name "NetworkMgr - Win32 Debug"
# Name "NetworkMgr - Win32 PS2 DVD Release"
# Name "NetworkMgr - Win32 PS2 DVD Debug"
# Name "NetworkMgr - Win32 PS2 Client Release"
# Name "NetworkMgr - Win32 PS2 Client Debug"
# Name "NetworkMgr - Win32 PS2 DevKit Release"
# Name "NetworkMgr - Win32 PS2 DevKit Debug"
# Begin Group "Connection"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Blowfish.cpp
# End Source File
# Begin Source File

SOURCE=.\Blowfish.h
# End Source File
# Begin Source File

SOURCE=.\ConnMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\ConnMgr.hpp
# End Source File
# Begin Source File

SOURCE=.\DebugTicker.cpp
# End Source File
# Begin Source File

SOURCE=.\DebugTicker.hpp
# End Source File
# Begin Source File

SOURCE=.\Dh.cpp
# End Source File
# Begin Source File

SOURCE=.\Dh.hpp
# End Source File
# Begin Source File

SOURCE=.\int.h
# End Source File
# Begin Source File

SOURCE=.\integer.cpp
# End Source File
# Begin Source File

SOURCE=.\integer.h
# End Source File
# Begin Source File

SOURCE=.\MatchMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\MatchMgr.hpp
# End Source File
# End Group
# Begin Group "ClientServer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ClientProxy.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientProxy.hpp
# End Source File
# Begin Source File

SOURCE=.\ClientProxyStates.cpp
# End Source File
# Begin Source File

SOURCE=.\GameClient.cpp
# End Source File
# Begin Source File

SOURCE=.\GameClient.hpp
# End Source File
# Begin Source File

SOURCE=.\GameClientStates.cpp
# End Source File
# Begin Source File

SOURCE=.\GameServer.cpp
# End Source File
# Begin Source File

SOURCE=.\GameServer.hpp
# End Source File
# Begin Source File

SOURCE=.\GameServerStates.cpp
# End Source File
# Begin Source File

SOURCE=.\GameState.cpp
# End Source File
# Begin Source File

SOURCE=.\GameState.hpp
# End Source File
# End Group
# Begin Group "GameMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\GameMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\GameMgr.hpp
# End Source File
# Begin Source File

SOURCE=.\GMMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\GMMgr.hpp
# End Source File
# Begin Source File

SOURCE=.\logic_Base.cpp
# End Source File
# Begin Source File

SOURCE=.\logic_Base.hpp
# End Source File
# Begin Source File

SOURCE=.\logic_Campaign.cpp
# End Source File
# Begin Source File

SOURCE=.\logic_Campaign.hpp
# End Source File
# Begin Source File

SOURCE=.\logic_DM.cpp
# End Source File
# Begin Source File

SOURCE=.\logic_DM.hpp
# End Source File
# Begin Source File

SOURCE=.\MsgMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\MsgMgr.hpp
# End Source File
# End Group
# Begin Group "NetObjects"

# PROP Default_Filter ""
# Begin Group "Defunct"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\NetPickup.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\NetPickup.hpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\NetPlayer.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\NetPlayer.hpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\NetRocket.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\NetRocket.hpp
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Source File

SOURCE=.\Blender.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\GhostNet.cpp
# End Source File
# Begin Source File

SOURCE=.\NetObj.cpp
# End Source File
# Begin Source File

SOURCE=.\NetObj.hpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\NetObjMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\NetObjMgr.hpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\Objects\PlayerNet.cpp
# End Source File
# End Group
# Begin Group "MoveMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Move.cpp
# End Source File
# Begin Source File

SOURCE=.\Move.hpp
# End Source File
# Begin Source File

SOURCE=.\MoveMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\MoveMgr.hpp
# End Source File
# End Group
# Begin Group "UpdateMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\UpdateMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\UpdateMgr.hpp
# End Source File
# End Group
# Begin Group "VoiceMgr"

# PROP Default_Filter ""
# Begin Group "Logitech"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Voice\Logitech\Include\liblgaud.h

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Logitech\Include\liblgcodec.h

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Logitech\Lib\liblgaud.lib

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

!ENDIF 

# End Source File
# End Group
# Begin Group "Codecs"

# PROP Default_Filter ""
# Begin Group "Speex"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Voice\Speex\arch.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\bits.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\cb_search.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\cb_search.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\cb_search_sse.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\exc_10_16_table.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\exc_10_32_table.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\exc_20_32_table.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\exc_5_256_table.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\exc_5_64_table.c
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\exc_8_128_table.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\filters.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\filters.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\filters_sse.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\fixed_arm.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\fixed_debug.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\fixed_generic.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\gain_table.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\gain_table_lbr.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\hexc_10_32_table.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\hexc_table.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\high_lsp_tables.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\jitter.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\lbr_48k_tables.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\lpc.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\lpc.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\lsp.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\lsp.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\lsp_tables_nb.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\ltp.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\ltp.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\ltp_sse.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\math_approx.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\math_approx.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\mdf.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\misc.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\misc.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\modes.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\modes.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\nb_celp.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\nb_celp.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\preprocess.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\quant_lsp.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\quant_lsp.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\sb_celp.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\sb_celp.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\smallft.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\smallft.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\speex.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\speex_bits.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\speex_callbacks.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\speex_callbacks.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\speex_echo.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\speex_header.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\speex_header.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\speex_jitter.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\speex_preprocess.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\speex_stereo.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\stack_alloc.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\stereo.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\vbr.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\vbr.h
# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\vq.c

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Speex\vq.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Voice\lpc10.cpp

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\lpc10.hpp
# End Source File
# Begin Source File

SOURCE=.\Voice\speex.cpp
# End Source File
# Begin Source File

SOURCE=.\Voice\speex.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Voice\Headset.hpp
# End Source File
# Begin Source File

SOURCE=.\Voice\Headset_PS2.cpp

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Headset_Stub.cpp

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\Headset_XBOX.cpp

!IF  "$(CFG)" == "NetworkMgr - Win32 Release"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 Debug"

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DVD Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "NetworkMgr - Win32 PS2 DevKit Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Voice\VoiceMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Voice\VoiceMgr.hpp
# End Source File
# Begin Source File

SOURCE=.\Voice\VoiceProxy.cpp
# End Source File
# Begin Source File

SOURCE=.\Voice\VoiceProxy.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\NetworkMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\NetworkMgr.hpp
# End Source File
# Begin Source File

SOURCE=.\ServerVersion.hpp
# End Source File
# End Target
# End Project
