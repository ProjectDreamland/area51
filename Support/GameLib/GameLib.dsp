# Microsoft Developer Studio Project File - Name="GameLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=GameLib - Win32 PS2 DevKit Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GameLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GameLib.mak" CFG="GameLib - Win32 PS2 DevKit Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GameLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "GameLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "GameLib - Win32 PS2 DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "GameLib - Win32 PS2 DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "GameLib - Win32 PS2 Client Release" (based on "Win32 (x86) Static Library")
!MESSAGE "GameLib - Win32 PS2 Client Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "GameLib - Win32 PS2 DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "GameLib - Win32 PS2 DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "GameLib"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GameLib - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /D "_LIB" /D "$(USERNAME)" /D "WIN32" /D "NDEBUG" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "GameLib - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /I "$(X)\Parsing" /D "_LIB" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /D "WIN32" /D "_DEBUG" /D "_MBCS" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 DVD Release"

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
# ADD CPP /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 DVD Debug"

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
# ADD CPP /Zi /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /D "X_DEBUG" /D "X_ASSERT" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 Client Release"

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
# ADD CPP /O1 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 Client Debug"

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
# ADD CPP /Zi /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 DevKit Release"

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
# ADD CPP /O2 /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "$(USERNAME)" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 DevKit Debug"

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
# ADD CPP /WX /Zi /Od /I "$(X)" /I "$(X)\x_files" /I "$(X)\Entropy" /I "$(X)\..\Support" /I "$(X)\Auxiliary" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /D "$(USERNAME)" /FD
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

# Name "GameLib - Win32 Release"
# Name "GameLib - Win32 Debug"
# Name "GameLib - Win32 PS2 DVD Release"
# Name "GameLib - Win32 PS2 DVD Debug"
# Name "GameLib - Win32 PS2 Client Release"
# Name "GameLib - Win32 PS2 Client Debug"
# Name "GameLib - Win32 PS2 DevKit Release"
# Name "GameLib - Win32 PS2 DevKit Debug"
# Begin Group "Animation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Animation\AnimData.hpp
# End Source File
# Begin Source File

SOURCE=..\Animation\AnimDecompress.cpp
# End Source File
# Begin Source File

SOURCE=..\Animation\AnimEvent.cpp
# End Source File
# Begin Source File

SOURCE=..\Animation\AnimGroup.cpp
# End Source File
# Begin Source File

SOURCE=..\Animation\AnimInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\Animation\AnimKeyData.cpp
# End Source File
# Begin Source File

SOURCE=..\Animation\AnimPlayer.cpp
# End Source File
# Begin Source File

SOURCE=..\Animation\AnimPlayer.hpp
# End Source File
# Begin Source File

SOURCE=..\Animation\AnimTrack.cpp
# End Source File
# Begin Source File

SOURCE=..\Animation\AnimTrack.hpp
# End Source File
# Begin Source File

SOURCE=..\Animation\BasePlayer.hpp
# End Source File
# Begin Source File

SOURCE=..\Animation\CharAnimPlayer.cpp
# End Source File
# Begin Source File

SOURCE=..\Animation\CharAnimPlayer.hpp
# End Source File
# Begin Source File

SOURCE=..\Animation\SMemMatrixCache.cpp
# End Source File
# Begin Source File

SOURCE=..\Animation\SMemMatrixCache.hpp
# End Source File
# End Group
# Begin Group "Actor"

# PROP Default_Filter ""
# Begin Group "Characters"

# PROP Default_Filter ""
# Begin Group "General"

# PROP Default_Filter ""
# Begin Group "Conversations"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Characters\Conversation_Packet.hpp
# End Source File
# End Group
# Begin Group "Base Character States"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Characters\BaseStates\Character_Alert_State.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\BaseStates\Character_Alert_State.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\BaseStates\Character_Attack_State.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\BaseStates\Character_Attack_State.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\BaseStates\Character_Cover_State.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\BaseStates\Character_Cover_State.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\BaseStates\Character_Death_State.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\BaseStates\Character_Death_State.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\BaseStates\Character_Flee_State.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\BaseStates\Character_Flee_State.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\BaseStates\Character_Follow_State.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\BaseStates\Character_Follow_State.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\BaseStates\Character_Idle_State.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\BaseStates\Character_Idle_State.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\BaseStates\Character_Search_State.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\BaseStates\Character_Search_State.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\BaseStates\Character_Trigger_State.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\BaseStates\Character_Trigger_State.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\BaseStates\Character_Turret_State.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\BaseStates\Character_Turret_State.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\CharacterState.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\CharacterState.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\Characters\AlertPackage.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\AStar.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\AStar.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\AStarNode.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\AStarNode.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\Character.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\factions.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\factions.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\God.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\GridHelper.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\GridHelper.hpp
# End Source File
# End Group
# Begin Group "Hazmat"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Characters\Soldiers\Hazmat.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\Soldiers\Hazmat.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\Soldiers\HazmatLoco.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\Soldiers\HazmatLoco.hpp
# End Source File
# End Group
# Begin Group "Gray"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Characters\Gray\Gray.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\Gray\Gray.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\Gray\Gray_Attack_State.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\Gray\Gray_Attack_State.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\Gray\GrayLoco.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\Gray\GrayLoco.hpp
# End Source File
# End Group
# Begin Group "Friendly Scientist"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Characters\Scientist\FriendlyScientist.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\Scientist\FriendlyScientist.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\Scientist\FriendlyScientistLoco.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\Scientist\FriendlyScientistLoco.hpp
# End Source File
# End Group
# Begin Group "TaskSystem"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Characters\TaskSystem\character_task.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\TaskSystem\character_task.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\TaskSystem\character_task_set.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\TaskSystem\character_task_set.hpp
# End Source File
# End Group
# Begin Group "Mutant Tank"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Characters\MutantTank\Mutant_Tank.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\MutantTank\Mutant_Tank.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\MutantTank\Mutant_Tank_Loco.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\MutantTank\Mutant_Tank_Loco.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\MutantTank\MutantTank_Attack_State.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\MutantTank\MutantTank_Attack_State.hpp
# End Source File
# End Group
# Begin Group "Grunt"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Characters\Grunt\Grunt.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\Grunt\Grunt.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\Grunt\Grunt_Attack_State.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\Grunt\Grunt_Attack_State.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\Grunt\GruntLoco.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\Grunt\GruntLoco.hpp
# End Source File
# End Group
# Begin Group "BlackOpps"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Characters\Soldiers\BlackOpps.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\Soldiers\BlackOpps.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\Soldiers\BlackOpps_Attack_State.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\Soldiers\BlackOpps_Attack_State.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\Soldiers\BlackOppsLoco.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\Soldiers\BlackOppsLoco.hpp
# End Source File
# End Group
# Begin Group "GenericNPC"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Characters\GenericNPC\GenericNPC.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\GenericNPC\GenericNPC.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\GenericNPC\GenericNPCLoco.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\GenericNPC\GenericNPCLoco.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\Characters\Character.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\CharacterGlobals.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\FloorProperties.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\FloorProperties.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\God.hpp
# End Source File
# Begin Source File

SOURCE=..\Characters\ResponseList.cpp
# End Source File
# Begin Source File

SOURCE=..\Characters\ResponseList.hpp
# End Source File
# End Group
# Begin Group "Player"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Objects\Ghost.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Ghost.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Player.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Player.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\Objects\Actor\Actor.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Actor\Actor.hpp
# End Source File
# Begin Source File

SOURCE=..\AI\AIMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\AI\AIMgr.hpp
# End Source File
# End Group
# Begin Group "DebugMenu"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Menu\DebugMenu.cpp
# End Source File
# Begin Source File

SOURCE=..\Menu\DebugMenu.hpp
# End Source File
# Begin Source File

SOURCE=..\Menu\DebugMenu_Main.cpp
# End Source File
# Begin Source File

SOURCE=..\Menu\DebugMenu_main.hpp
# End Source File
# End Group
# Begin Group "Decals"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Decals\DecalDefinition.cpp
# End Source File
# Begin Source File

SOURCE=..\Decals\DecalDefinition.hpp
# End Source File
# Begin Source File

SOURCE=..\Decals\DecalLoader.cpp
# End Source File
# Begin Source File

SOURCE=..\Decals\DecalMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Decals\DecalMgr.hpp
# End Source File
# Begin Source File

SOURCE=..\Decals\DecalMgr_StaticDecals.cpp
# End Source File
# Begin Source File

SOURCE=..\Decals\DecalPackage.cpp
# End Source File
# Begin Source File

SOURCE=..\Decals\DecalPackage.hpp
# End Source File
# End Group
# Begin Group "Font"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Font\font.cpp
# End Source File
# Begin Source File

SOURCE=..\Font\font.hpp
# End Source File
# End Group
# Begin Group "HUd"

# PROP Default_Filter ""
# Begin Group "Controllers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Hud\HotkeyItemController.cpp
# End Source File
# Begin Source File

SOURCE=..\Hud\HotkeyItemController.hpp
# End Source File
# Begin Source File

SOURCE=..\Hud\HudControllerBase.cpp
# End Source File
# Begin Source File

SOURCE=..\Hud\HudControllerBase.hpp
# End Source File
# Begin Source File

SOURCE=..\Hud\HudItemController.cpp
# End Source File
# Begin Source File

SOURCE=..\Hud\HudItemController.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\Hud\Focus_Inst.cpp
# End Source File
# Begin Source File

SOURCE=..\Hud\Focus_Inst.hpp
# End Source File
# Begin Source File

SOURCE=..\Hud\HUD.cpp
# End Source File
# Begin Source File

SOURCE=..\Hud\HUD.hpp
# End Source File
# Begin Source File

SOURCE=..\Hud\RiftHUD.cpp
# End Source File
# Begin Source File

SOURCE=..\Hud\RiftHUD.hpp
# End Source File
# Begin Source File

SOURCE=..\Hud\SmartSprite.cpp
# End Source File
# Begin Source File

SOURCE=..\Hud\SmartSprite.hpp
# End Source File
# End Group
# Begin Group "Inventory"

# PROP Default_Filter ""
# Begin Group "InventoryObjects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Inventory\CollectableAntiMutagen.cpp
# End Source File
# Begin Source File

SOURCE=..\Inventory\CollectableAntiMutagen.hpp
# End Source File
# Begin Source File

SOURCE=..\Inventory\CollectableHealthPack.cpp
# End Source File
# Begin Source File

SOURCE=..\Inventory\CollectableHealthPack.hpp
# End Source File
# Begin Source File

SOURCE=..\Inventory\CollectableKeyCard.cpp
# End Source File
# Begin Source File

SOURCE=..\Inventory\CollectableKeyCard.hpp
# End Source File
# Begin Source File

SOURCE=..\Inventory\GenericItem.cpp
# End Source File
# Begin Source File

SOURCE=..\Inventory\GenericItem.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\Inventory\Inventory.cpp
# End Source File
# Begin Source File

SOURCE=..\Inventory\Inventory.hpp
# End Source File
# Begin Source File

SOURCE=..\Inventory\InventoryBitmapResource.cpp
# End Source File
# Begin Source File

SOURCE=..\Inventory\InventoryBitmapResource.hpp
# End Source File
# Begin Source File

SOURCE=..\Inventory\InventoryItem.cpp
# End Source File
# Begin Source File

SOURCE=..\Inventory\InventoryItem.hpp
# End Source File
# End Group
# Begin Group "Loco"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Locomotion\CharacterPhysics.cpp
# End Source File
# Begin Source File

SOURCE=..\Locomotion\CharacterPhysics.hpp
# End Source File
# Begin Source File

SOURCE=..\Loco\Loco.cpp
# End Source File
# Begin Source File

SOURCE=..\Loco\Loco.hpp
# End Source File
# Begin Source File

SOURCE=..\Loco\LocoAdditiveController.cpp
# End Source File
# Begin Source File

SOURCE=..\Loco\LocoAdditiveController.hpp
# End Source File
# Begin Source File

SOURCE=..\Loco\LocoAimController.cpp
# End Source File
# Begin Source File

SOURCE=..\Loco\LocoAimController.hpp
# End Source File
# Begin Source File

SOURCE=..\Loco\LocoAnimController.cpp
# End Source File
# Begin Source File

SOURCE=..\Loco\LocoAnimController.hpp
# End Source File
# Begin Source File

SOURCE=..\Loco\LocoCharAnimPlayer.cpp
# End Source File
# Begin Source File

SOURCE=..\Loco\LocoCharAnimPlayer.hpp
# End Source File
# Begin Source File

SOURCE=..\Loco\LocoEyeController.cpp
# End Source File
# Begin Source File

SOURCE=..\Loco\LocoEyeController.hpp
# End Source File
# Begin Source File

SOURCE=..\Loco\LocoLipSyncController.cpp
# End Source File
# Begin Source File

SOURCE=..\Loco\LocoLipSyncController.hpp
# End Source File
# Begin Source File

SOURCE=..\Loco\LocoMaskController.cpp
# End Source File
# Begin Source File

SOURCE=..\Loco\LocoMaskController.hpp
# End Source File
# Begin Source File

SOURCE=..\Loco\LocoMotionController.cpp
# End Source File
# Begin Source File

SOURCE=..\Loco\LocoMotionController.hpp
# End Source File
# Begin Source File

SOURCE=..\Loco\LocoPhysics.cpp
# End Source File
# Begin Source File

SOURCE=..\Loco\LocoPhysics.hpp
# End Source File
# Begin Source File

SOURCE=..\Loco\LocoRagdollController.cpp
# End Source File
# Begin Source File

SOURCE=..\Loco\LocoRagdollController.hpp
# End Source File
# Begin Source File

SOURCE=..\Loco\LocoUtil.cpp
# End Source File
# Begin Source File

SOURCE=..\Loco\LocoUtil.hpp
# End Source File
# Begin Source File

SOURCE=..\Locomotion\RiftyPhysics.cpp
# End Source File
# Begin Source File

SOURCE=..\Locomotion\RiftyPhysics.hpp
# End Source File
# End Group
# Begin Group "Misc Utils"

# PROP Default_Filter ""
# Begin Group "Guid"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\xCore\Auxiliary\MiscUtils\Guid.cpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\Auxiliary\MiscUtils\Guid.hpp
# End Source File
# End Group
# Begin Group "Misc Utils A51"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\MiscUtils\CombatAimer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\MiscUtils\CombatAimer.hpp
# End Source File
# Begin Source File

SOURCE=..\..\MiscUtils\SimpleUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\MiscUtils\SimpleUtils.hpp
# End Source File
# Begin Source File

SOURCE=..\..\MiscUtils\TrajectoryGenerator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\MiscUtils\TrajectoryGenerator.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\xCore\Auxiliary\MiscUtils\dictionary.cpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\Auxiliary\MiscUtils\dictionary.hpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\Auxiliary\MiscUtils\fileio.hpp
# End Source File
# Begin Source File

SOURCE=..\ManagerRegistration.cpp
# End Source File
# Begin Source File

SOURCE=..\ManagerRegistration.hpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\Auxiliary\MiscUtils\mem_stream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\Auxiliary\MiscUtils\mem_stream.hpp
# End Source File
# Begin Source File

SOURCE=..\..\MiscUtils\PriorityQueue.hpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\Auxiliary\MiscUtils\Property.cpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\Auxiliary\MiscUtils\Property.hpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\Auxiliary\MiscUtils\PropertyEnum.hpp
# End Source File
# Begin Source File

SOURCE=..\..\xCore\Auxiliary\MiscUtils\RTTI.hpp
# End Source File
# Begin Source File

SOURCE=.\StatsMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\StatsMgr.hpp
# End Source File
# End Group
# Begin Group "Objects"

# PROP Default_Filter ""
# Begin Group "Pickups"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Objects\BoostPickup.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\BoostPickup.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\DefencePickup.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\DefencePickup.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\HealthPickup.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\HealthPickup.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Pickup.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Pickup.hpp
# End Source File
# End Group
# Begin Group "Debris"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Debris\debris.cpp

!IF  "$(CFG)" == "GameLib - Win32 Release"

!ELSEIF  "$(CFG)" == "GameLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 DevKit Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Debris\debris.hpp

!IF  "$(CFG)" == "GameLib - Win32 Release"

!ELSEIF  "$(CFG)" == "GameLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 DevKit Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Debris\debris_glass.cpp
# End Source File
# Begin Source File

SOURCE=..\Debris\debris_glass.hpp
# End Source File
# Begin Source File

SOURCE=..\Debris\debris_mgr.cpp

!IF  "$(CFG)" == "GameLib - Win32 Release"

!ELSEIF  "$(CFG)" == "GameLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 DevKit Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Debris\debris_mgr.hpp

!IF  "$(CFG)" == "GameLib - Win32 Release"

!ELSEIF  "$(CFG)" == "GameLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 Client Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 DevKit Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\debris_rigid.cpp
# End Source File
# Begin Source File

SOURCE=..\Debris\debris_rigid.hpp
# End Source File
# End Group
# Begin Group "Ladders"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Objects\Ladders\Ladder_Field.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Ladders\Ladder_Field.hpp
# End Source File
# End Group
# Begin Group "Spawner"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Objects\Spawner\SpawnerObject.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Spawner\SpawnerObject.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\Objects\Animation_obj.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Animation_obj.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\AnimSurface.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\AnimSurface.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Camera.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Camera.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\ClothObject.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\ClothObject.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Controller.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Controller.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Coupler.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Coupler.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\DamageField.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\DamageField.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\DeadBody.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\DeadBody.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\DestructibleObj.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\DestructibleObj.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Door.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Door.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Event.hpp
# End Source File
# Begin Source File

SOURCE=..\Sound\EventSoundEmitter.cpp
# End Source File
# Begin Source File

SOURCE=..\Sound\EventSoundEmitter.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\FocusObject.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\FocusObject.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\GlassSurface.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\GlassSurface.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\HudObject.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\HudObject.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\InputSetting.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\InputSetting.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\LevelSettings.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\LevelSettings.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\LightObject.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\LightObject.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Manipulator.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Manipulator.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\marker_object.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\marker_object.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\MusicLogic.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\MusicLogic.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\NavPoint.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\NavPoint.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\notepad_object.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\notepad_object.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\object.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\object.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Pain.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Pain.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\ParticleEmiter.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\ParticleEmiter.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\ParticleEventEmitter.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\ParticleEventEmitter.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Path.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Path.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Pip.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Pip.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\PlayerLoco.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\PlayerLoco.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\PlaySurface.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\PlaySurface.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Portal.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Portal.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Projector.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Projector.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\PropSurface.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\PropSurface.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\ProxyPlaySurface.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\ProxyPlaySurface.hpp
# End Source File
# Begin Source File

SOURCE=.\RigidGeomCollision.cpp
# End Source File
# Begin Source File

SOURCE=.\RigidGeomCollision.hpp
# End Source File
# Begin Source File

SOURCE=..\Sound\SimpleSoundEmitter.cpp
# End Source File
# Begin Source File

SOURCE=..\Sound\SimpleSoundEmitter.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\SkinPropSurface.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\SkinPropSurface.hpp
# End Source File
# Begin Source File

SOURCE=..\Sound\SoundEmitter.cpp
# End Source File
# Begin Source File

SOURCE=..\Sound\SoundEmitter.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\SpawnPoint.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\SpawnPoint.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\ThirdPersonCamera.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\ThirdPersonCamera.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Tracker.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Tracker.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Turret.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Turret.hpp
# End Source File
# End Group
# Begin Group "Obj_mgr"

# PROP Default_Filter ""
# Begin Group "SpatialDBase"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\SpatialDBase\Spatial_user.hpp
# End Source File
# Begin Source File

SOURCE=..\SpatialDBase\SpatialDBase.cpp
# End Source File
# Begin Source File

SOURCE=..\SpatialDBase\SpatialDBase_inline.hpp
# End Source File
# End Group
# Begin Group "ResourceMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\ResourceMgr\inline_ResourceMgr.hpp
# End Source File
# Begin Source File

SOURCE=..\ResourceMgr\ResourceMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\ResourceMgr\ResourceMgr.hpp
# End Source File
# End Group
# Begin Group "CollisionMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\CollisionMgr\CollisionMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\CollisionMgr\CollisionMgr.hpp
# End Source File
# Begin Source File

SOURCE=..\CollisionMgr\CollisionMgr_Private.hpp
# End Source File
# Begin Source File

SOURCE=..\CollisionMgr\CollisionPrimatives.cpp
# End Source File
# Begin Source File

SOURCE=..\CollisionMgr\CollisionPrimatives.hpp
# End Source File
# Begin Source File

SOURCE=..\CollisionMgr\PolyCache.cpp
# End Source File
# Begin Source File

SOURCE=..\CollisionMgr\PolyCache.hpp
# End Source File
# End Group
# Begin Group "InputMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\InputMgr\GamePad.cpp
# End Source File
# Begin Source File

SOURCE=..\InputMgr\GamePad.hpp
# End Source File
# Begin Source File

SOURCE=..\InputMgr\InputMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\InputMgr\InputMgr.hpp
# End Source File
# End Group
# Begin Group "ZoneMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\ZoneMgr\ZoneMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\ZoneMgr\ZoneMgr.hpp
# End Source File
# End Group
# Begin Group "TemplateMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\TemplateMgr\TemplateMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\TemplateMgr\TemplateMgr.hpp
# End Source File
# End Group
# Begin Group "PlaySurfaceMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\PlaySurfaceMgr\PlaySurfaceDBase.cpp
# End Source File
# Begin Source File

SOURCE=..\PlaySurfaceMgr\PlaySurfaceMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\PlaySurfaceMgr\PlaySurfaceMgr.hpp
# End Source File
# End Group
# Begin Group "GameTextMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\GameTextMgr\GameTextMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\GameTextMgr\GameTextMgr.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\Dictionary\Global_Dictionary.cpp
# End Source File
# Begin Source File

SOURCE=..\Dictionary\Global_Dictionary.hpp
# End Source File
# Begin Source File

SOURCE=..\Globals\Global_Variables_Manager.cpp
# End Source File
# Begin Source File

SOURCE=..\Globals\Global_Variables_Manager.hpp
# End Source File
# Begin Source File

SOURCE=..\Obj_mgr\Obj_Mgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Obj_mgr\obj_mgr.hpp
# End Source File
# End Group
# Begin Group "Navigation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\navigation\ConnectionZoneMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\navigation\ConnectionZoneMgr.hpp
# End Source File
# Begin Source File

SOURCE=..\navigation\CoverNode.cpp
# End Source File
# Begin Source File

SOURCE=..\navigation\CoverNode.hpp
# End Source File
# Begin Source File

SOURCE=..\Navigation\nav_map.cpp
# End Source File
# Begin Source File

SOURCE=..\Navigation\Nav_Map.hpp
# End Source File
# Begin Source File

SOURCE=..\navigation\ng_connection2.cpp
# End Source File
# Begin Source File

SOURCE=..\navigation\ng_connection2.hpp
# End Source File
# Begin Source File

SOURCE=..\navigation\ng_node2.cpp
# End Source File
# Begin Source File

SOURCE=..\navigation\ng_node2.hpp
# End Source File
# End Group
# Begin Group "Periodical"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Periodical\Periodical.cpp
# End Source File
# Begin Source File

SOURCE=..\Periodical\Periodical.hpp
# End Source File
# Begin Source File

SOURCE=..\Periodical\Periodical_example.hpp

!IF  "$(CFG)" == "GameLib - Win32 Release"

!ELSEIF  "$(CFG)" == "GameLib - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 DVD Release"

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 DVD Debug"

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 Client Release"

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 Client Debug"

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 DevKit Release"

!ELSEIF  "$(CFG)" == "GameLib - Win32 PS2 DevKit Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Periodical\Periodical_mgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Periodical\Periodical_mgr.hpp
# End Source File
# End Group
# Begin Group "Render"

# PROP Default_Filter "*.cpp *.hpp"
# Begin Source File

SOURCE=..\Render\editor_icons.cpp
# End Source File
# Begin Source File

SOURCE=..\Render\editor_icons.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Render\PostEffectMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Render\PostEffectMgr.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Render\RenderInst.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Render\RenderInst.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Render\RigidInst.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Render\RigidInst.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Render\SkinInst.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\Render\SkinInst.hpp
# End Source File
# End Group
# Begin Group "Sound"

# PROP Default_Filter ""
# End Group
# Begin Group "TriggerEx"

# PROP Default_Filter ""
# Begin Group "ActionsEx"

# PROP Default_Filter ""
# Begin Group "AI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_ai_attack_guid.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_ai_attack_guid.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_ai_base.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_ai_base.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_ai_dialog_line.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_ai_dialog_line.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_ai_inventory.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_ai_inventory.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_ai_lookat_guid.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_ai_lookat_guid.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_ai_nav_activation.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_ai_nav_activation.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_ai_pathto_guid.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_ai_pathto_guid.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_ai_play_anim.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_ai_play_anim.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_ai_searchto_guid.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_ai_searchto_guid.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_affect_global.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_affect_global.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_create_template.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_create_template.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_display_text.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_display_text.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_door_logic.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_door_logic.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_load_level.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_load_level.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_music_intensity.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_music_intensity.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_object_activation.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_object_activation.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_object_damage.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_object_damage.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_object_destroy.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_object_destroy.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_object_move.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_object_move.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_object_move_relative.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_object_move_relative.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_player_camera_shake.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_player_camera_shake.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_player_inventory.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_player_inventory.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_portal_activate.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_portal_activate.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_set_property.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Actions\action_set_property.hpp
# End Source File
# End Group
# Begin Group "ConditionsEx"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\TriggerEx\Conditions\condition_check_global.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Conditions\condition_check_global.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Conditions\condition_check_property.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Conditions\condition_check_property.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Conditions\condition_object_exists.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Conditions\condition_object_exists.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Conditions\condition_player_button_state.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Conditions\condition_player_button_state.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Conditions\condition_player_has_item.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Conditions\condition_player_has_item.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Conditions\condition_random_chance.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Conditions\condition_random_chance.hpp
# End Source File
# End Group
# Begin Group "Affecters"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\TriggerEx\Affecters\conditional_affecter.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Affecters\conditional_affecter.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Affecters\object_affecter.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Affecters\object_affecter.hpp
# End Source File
# End Group
# Begin Group "Meta"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\TriggerEx\Meta\trigger_meta_base.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Meta\trigger_meta_base.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Meta\trigger_meta_block.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Meta\trigger_meta_block.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Meta\trigger_meta_breakpoint.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Meta\trigger_meta_breakpoint.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Meta\trigger_meta_delay.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Meta\trigger_meta_delay.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Meta\trigger_meta_goto.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Meta\trigger_meta_goto.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Meta\trigger_meta_label.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\Meta\trigger_meta_label.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\TriggerEx\TriggerEx_Actions.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\TriggerEx_Actions.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\TriggerEx_Conditionals.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\TriggerEx_Conditionals.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\TriggerEx_Manager.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\TriggerEx_Manager.hpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\TriggerEx_Object.cpp
# End Source File
# Begin Source File

SOURCE=..\TriggerEx\TriggerEx_Object.hpp
# End Source File
# End Group
# Begin Group "Havok"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Havok\Havok.cpp
# End Source File
# Begin Source File

SOURCE=..\Havok\Havok.hpp
# End Source File
# End Group
# Begin Group "Tracers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Tracers\TracerMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Tracers\TracerMgr.hpp
# End Source File
# End Group
# Begin Group "Cloth"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Cloth\Cloth.cpp
# End Source File
# Begin Source File

SOURCE=..\Cloth\Cloth.hpp
# End Source File
# End Group
# Begin Group "ragdoll"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Ragdoll\DistRule.cpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\DistRule.hpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\GeomBone.cpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\GeomBone.hpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\GrayRagdoll.cpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\GrayRagdoll.hpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\GruntRagdoll.cpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\GruntRagdoll.hpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\HazmatRagdoll.cpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\HazmatRagdoll.hpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\HumanRagdoll.cpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\HumanRagdoll.hpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\Mutant3Ragdoll.cpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\Mutant3Ragdoll.hpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\MutantTankRagdoll.cpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\MutantTankRagdoll.hpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\Particle.cpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\Particle.hpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\PlayerRagdoll.cpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\PlayerRagdoll.hpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\Ragdoll.cpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\Ragdoll.hpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\ScientistRagdoll.cpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\ScientistRagdoll.hpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\StickBone.cpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\StickBone.hpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\VerletCollision.cpp
# End Source File
# Begin Source File

SOURCE=..\Ragdoll\VerletCollision.hpp
# End Source File
# End Group
# Begin Group "Weapons"

# PROP Default_Filter ""
# Begin Group "Weapon Objects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Objects\WeaponDesertEagle.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\WeaponDesertEagle.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\WeaponFragGrenade.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\WeaponFragGrenade.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\WeaponGauss.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\WeaponGauss.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\WeaponGravCharge.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\WeaponGravCharge.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\WeaponMHG.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\WeaponMHG.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\WeaponMSN.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\WeaponMSN.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\WeaponShotgun.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\WeaponShotgun.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\WeaponSMP.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\WeaponSMP.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\WeaponSniper.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\WeaponSniper.hpp
# End Source File
# End Group
# Begin Group "Projectile Objects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Objects\GravChargeProjectile.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\GravChargeProjectile.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\GrenadeProjectile.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\GrenadeProjectile.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\ProjectileBullett.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\ProjectileBullett.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\ProjectileEnergy.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\ProjectileEnergy.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\ProjectileExplosiveBullett.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\ProjectileExplosiveBullett.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\ProjectileSeeker.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\ProjectileSeeker.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\Objects\BaseProjectile.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\BaseProjectile.hpp
# End Source File
# Begin Source File

SOURCE=..\Objects\NewWeapon.cpp
# End Source File
# Begin Source File

SOURCE=..\Objects\NewWeapon.hpp
# End Source File
# End Group
# Begin Group "Dialogs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Dialogs\dlg_EndPause.cpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_EndPause.hpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_LevelSelect.cpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_LevelSelect.hpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_MainMenu.cpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_MainMenu.hpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_MainOptions.cpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_MainOptions.hpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_MCMessage.cpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_MCMessage.hpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_MultiOptions.cpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_MultiOptions.hpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_MultiPlayer.cpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_MultiPlayer.hpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_OnlineConnect.cpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_OnlineConnect.hpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_OnlineHost.cpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_OnlineHost.hpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_OnlineJoin.cpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_OnlineJoin.hpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_OnlineLevelSelect.cpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_OnlineLevelSelect.hpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_OnlineMain.cpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_OnlineMain.hpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_OnlinePlayers.cpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_OnlinePlayers.hpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_PauseMain.cpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_PauseMain.hpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_PopUp.cpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_PopUp.hpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_PressStart.cpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_PressStart.hpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_ProfileAV.cpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_ProfileAV.hpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_ProfileControls.cpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_ProfileControls.hpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_ProfileName.cpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_ProfileName.hpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_ProfileOptions.cpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_ProfileOptions.hpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_ProfileSelect.cpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_ProfileSelect.hpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_StartGame.cpp
# End Source File
# Begin Source File

SOURCE=..\Dialogs\dlg_StartGame.hpp
# End Source File
# End Group
# Begin Group "StateMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\StateMgr\StateMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\StateMgr\StateMgr.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\BinLevel.cpp
# End Source File
# Begin Source File

SOURCE=.\BinLevel.hpp
# End Source File
# Begin Source File

SOURCE=.\Level.cpp
# End Source File
# Begin Source File

SOURCE=.\Level.hpp
# End Source File
# Begin Source File

SOURCE=.\Link.cpp
# End Source File
# Begin Source File

SOURCE=.\Link.hpp
# End Source File
# Begin Source File

SOURCE=..\SpatialDBase\SpatialDBase.hpp
# End Source File
# End Target
# End Project
