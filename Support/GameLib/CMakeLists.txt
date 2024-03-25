set(SUPPORT_PATH "${CMAKE_SOURCE_DIR}/Support")
set(GAMELIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

# Root GameLib sources
file(GLOB GAMELIB_SRC
	"${GAMELIB_PATH}/*.cpp"
	"${GAMELIB_PATH}/*.hpp"
)

# Support Root
set(GAMELIB_ROOT_SRC
	"${SUPPORT_PATH}/ManagerRegistration.cpp"
	"${SUPPORT_PATH}/ManagerRegistration.hpp"
)

# AI
file(GLOB_RECURSE GAMELIB_AI_SRC
	"${SUPPORT_PATH}/AI/*.cpp"
	"${SUPPORT_PATH}/AI/*.hpp"
)

# Animation
file(GLOB_RECURSE GAMELIB_ANIMATION_SRC
	"${SUPPORT_PATH}/Animation/*.cpp"
	"${SUPPORT_PATH}/Animation/*.hpp"
)

# Characters
file(GLOB_RECURSE GAMELIB_CHARACTERS_SRC
	"${SUPPORT_PATH}/Characters/*.cpp"
	"${SUPPORT_PATH}/Characters/*.hpp"
)

# CheckPointMgr
file(GLOB_RECURSE GAMELIB_CHECKPOINTMGR_SRC
	"${SUPPORT_PATH}/CheckPointMgr/*.cpp"
	"${SUPPORT_PATH}/CheckPointMgr/*.hpp"
)

# Cloth
file(GLOB_RECURSE GAMELIB_CLOTH_SRC
	"${SUPPORT_PATH}/Cloth/*.cpp"
	"${SUPPORT_PATH}/Cloth/*.hpp"
)

# CollisionMgr
file(GLOB_RECURSE GAMELIB_COLLISIONMGR_SRC
	"${SUPPORT_PATH}/CollisionMgr/*.cpp"
	"${SUPPORT_PATH}/CollisionMgr/*.hpp"
)

# Configuration
file(GLOB_RECURSE GAMELIB_CONFIGURATION_SRC
	"${SUPPORT_PATH}/Configuration/*.cpp"
	"${SUPPORT_PATH}/Configuration/*.hpp"
)

# DataVault
file(GLOB_RECURSE GAMELIB_DATAVAULT_SRC
	"${SUPPORT_PATH}/DataVault/*.cpp"
	"${SUPPORT_PATH}/DataVault/*.hpp"
)

# Debris
file(GLOB_RECURSE GAMELIB_DEBRIS_SRC
	"${SUPPORT_PATH}/Debris/*.cpp"
	"${SUPPORT_PATH}/Debris/*.hpp"
)

# Decals
file(GLOB_RECURSE GAMELIB_DECALS_SRC
	"${SUPPORT_PATH}/Decals/*.cpp"
	"${SUPPORT_PATH}/Decals/*.hpp"
)

# Dictionary
file(GLOB_RECURSE GAMELIB_DICTIONARY_SRC
	"${SUPPORT_PATH}/Dictionary/*.cpp"
	"${SUPPORT_PATH}/Dictionary/*.hpp"
)

# GameTextMgr
file(GLOB_RECURSE GAMELIB_GAMETEXTMGR_SRC
	"${SUPPORT_PATH}/GameTextMgr/*.cpp"
	"${SUPPORT_PATH}/GameTextMgr/*.hpp"
)

# Globals
file(GLOB_RECURSE GAMELIB_GLOBALS_SRC
	"${SUPPORT_PATH}/Globals/*.cpp"
	"${SUPPORT_PATH}/Globals/*.hpp"
)

# InputMgr
file(GLOB_RECURSE GAMELIB_INPUTMGR_SRC
	"${SUPPORT_PATH}/InputMgr/*.cpp"
	"${SUPPORT_PATH}/InputMgr/*.hpp"
)

# Inventory
file(GLOB_RECURSE GAMELIB_INVENTORY_SRC
	"${SUPPORT_PATH}/Inventory/*.cpp"
	"${SUPPORT_PATH}/Inventory/*.hpp"
)

# Loco
file(GLOB_RECURSE GAMELIB_LOCO_SRC
	"${SUPPORT_PATH}/Loco/*.cpp"
	"${SUPPORT_PATH}/Loco/*.hpp"
)

# Locomotion
file(GLOB_RECURSE GAMELIB_LOCOMOTION_SRC
	"${SUPPORT_PATH}/Locomotion/*.cpp"
	"${SUPPORT_PATH}/Locomotion/*.hpp"
)

# MemCardMgr
file(GLOB GAMELIB_MEMCARDMGR_SRC
	"${SUPPORT_PATH}/MemCardMgr/*.cpp"
	"${SUPPORT_PATH}/MemCardMgr/*.hpp"
)

# Menu
file(GLOB GAMELIB_MENU_SRC
	"${SUPPORT_PATH}/Menu/*.cpp"
	"${SUPPORT_PATH}/Menu/*.hpp"
)

# MusicStateMgr
file(GLOB GAMELIB_MUSICSTATEMGR_SRC
	"${SUPPORT_PATH}/MusicStateMgr/*.cpp"
	"${SUPPORT_PATH}/MusicStateMgr/*.hpp"
)

# Navigation
file(GLOB GAMELIB_NAVIGATION_SRC
	"${SUPPORT_PATH}/navigation/*.cpp"
	"${SUPPORT_PATH}/navigation/*.hpp"
)

# Object Manager
file(GLOB GAMELIB_OBJMGR_SRC
	"${SUPPORT_PATH}/Obj_mgr/*.cpp"
	"${SUPPORT_PATH}/Obj_mgr/*.hpp"
)

# Objects
file(GLOB_RECURSE GAMELIB_OBJECTS_SRC
	"${SUPPORT_PATH}/Objects/*.cpp"
	"${SUPPORT_PATH}/Objects/*.hpp"
)

# OccluderMgr
file(GLOB_RECURSE GAMELIB_OCCLUDERMGR_SRC
	"${SUPPORT_PATH}/OccluderMgr/*.cpp"
	"${SUPPORT_PATH}/OccluderMgr/*.hpp"
)

# PainMgr
file(GLOB_RECURSE GAMELIB_PAINMGR_SRC
	"${SUPPORT_PATH}/PainMgr/*.cpp"
	"${SUPPORT_PATH}/PainMgr/*.hpp"
)

# PerceptionMgr
file(GLOB_RECURSE GAMELIB_PERCEPTIONMGR_SRC
	"${SUPPORT_PATH}/PerceptionMgr/*.cpp"
	"${SUPPORT_PATH}/PerceptionMgr/*.hpp"
)

# PhysicsMgr
file(GLOB_RECURSE GAMELIB_PHYSICSMGR_SRC
	"${SUPPORT_PATH}/PhysicsMgr/*.cpp"
	"${SUPPORT_PATH}/PhysicsMgr/*.hpp"
)

# PlaySurfaceMgr
file(GLOB_RECURSE GAMELIB_PLAYSURFACEMGR_SRC
	"${SUPPORT_PATH}/PlaySurfaceMgr/*.cpp"
	"${SUPPORT_PATH}/PlaySurfaceMgr/*.hpp"
)

# Ragdoll
file(GLOB_RECURSE GAMELIB_RAGDOLL_SRC
	"${SUPPORT_PATH}/Ragdoll/*.cpp"
	"${SUPPORT_PATH}/Ragdoll/*.hpp"
)

# ResourceMgr
file(GLOB_RECURSE GAMELIB_RESOURCEMGR_SRC
	"${SUPPORT_PATH}/ResourceMgr/*.cpp"
	"${SUPPORT_PATH}/ResourceMgr/*.hpp"
)

# Sound
file(GLOB_RECURSE GAMELIB_SOUND_SRC
	"${SUPPORT_PATH}/Sound/*.cpp"
	"${SUPPORT_PATH}/Sound/*.hpp"
)

# SpatialDBase
file(GLOB_RECURSE GAMELIB_SPATIALDBASE_SRC
	"${SUPPORT_PATH}/SpatialDBase/*.cpp"
	"${SUPPORT_PATH}/SpatialDBase/*.hpp"
)

# StateMgr
file(GLOB_RECURSE GAMELIB_STATEMGR_SRC
	"${SUPPORT_PATH}/StateMgr/*.cpp"
	"${SUPPORT_PATH}/StateMgr/*.hpp"
)

# StringMgr
file(GLOB_RECURSE GAMELIB_STRINGMGR_SRC
	"${SUPPORT_PATH}/StringMgr/*.cpp"
	"${SUPPORT_PATH}/StringMgr/*.hpp"
)

# TemplateMgr
file(GLOB_RECURSE GAMELIB_TEMPLATEMGR_SRC
	"${SUPPORT_PATH}/TemplateMgr/*.cpp"
	"${SUPPORT_PATH}/TemplateMgr/*.hpp"
)

# Tracers
file(GLOB_RECURSE GAMELIB_TRACERS_SRC
	"${SUPPORT_PATH}/Tracers/*.cpp"
	"${SUPPORT_PATH}/Tracers/*.hpp"
)

# TriggerEx
file(GLOB_RECURSE GAMELIB_TRIGGEREX_SRC
	"${SUPPORT_PATH}/TriggerEx/*.cpp"
	"${SUPPORT_PATH}/TriggerEx/*.hpp"
)

# TweakMgr
file(GLOB_RECURSE GAMELIB_TWEAKMGR_SRC
	"${SUPPORT_PATH}/TweakMgr/*.cpp"
	"${SUPPORT_PATH}/TweakMgr/*.hpp"
)

# ZoneMgr
file(GLOB_RECURSE GAMELIB_ZONEMGR_SRC
	"${SUPPORT_PATH}/ZoneMgr/*.cpp"
	"${SUPPORT_PATH}/ZoneMgr/*.hpp"
)


# Source Groupes
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_ROOT_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_AI_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_ANIMATION_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_CHARACTERS_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_CHECKPOINTMGR_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_CLOTH_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_COLLISIONMGR_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_CONFIGURATION_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_DATAVAULT_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_DEBRIS_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_DECALS_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_DICTIONARY_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_GAMETEXTMGR_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_GLOBALS_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_INPUTMGR_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_INVENTORY_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_LOCO_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_LOCOMOTION_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_MEMCARDMGR_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_MENU_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_MUSICSTATEMGR_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_NAVIGATION_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_OBJMGR_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_OBJECTS_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_OCCLUDERMGR_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_PAINMGR_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_PERCEPTIONMGR_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_PHYSICSMGR_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_PLAYSURFACEMGR_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_RAGDOLL_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_RESOURCEMGR_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_SOUND_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_SPATIALDBASE_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_STATEMGR_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_STRINGMGR_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_TEMPLATEMGR_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_TRACERS_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_TRIGGEREX_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_TWEAKMGR_SRC})
source_group(TREE ${SUPPORT_PATH} FILES ${GAMELIB_ZONEMGR_SRC})

add_library(GameLib STATIC 	${GAMELIB_SRC}
							${GAMELIB_ROOT_SRC}
							${GAMELIB_AI_SRC}
							${GAMELIB_ANIMATION_SRC}
							${GAMELIB_CHARACTERS_SRC}
							${GAMELIB_CHECKPOINTMGR_SRC}
							${GAMELIB_CLOTH_SRC}
							${GAMELIB_COLLISIONMGR_SRC}
							${GAMELIB_CONFIGURATION_SRC}
							${GAMELIB_DATAVAULT_SRC}
							${GAMELIB_DEBRIS_SRC}
							${GAMELIB_DECALS_SRC}
							${GAMELIB_DICTIONARY_SRC}
							${GAMELIB_GAMETEXTMGR_SRC}
							${GAMELIB_GLOBALS_SRC}
							${GAMELIB_INPUTMGR_SRC}
							${GAMELIB_INVENTORY_SRC}
							${GAMELIB_LOCO_SRC}
							${GAMELIB_LOCOMOTION_SRC}
							${GAMELIB_MEMCARDMGR_SRC}
							${GAMELIB_MENU_SRC}
							${GAMELIB_MUSICSTATEMGR_SRC}
							${GAMELIB_NAVIGATION_SRC}
							${GAMELIB_OBJMGR_SRC}
							${GAMELIB_OBJECTS_SRC}
							${GAMELIB_OCCLUDERMGR_SRC}
							${GAMELIB_PAINMGR_SRC}
							${GAMELIB_PERCEPTIONMGR_SRC}
							${GAMELIB_PHYSICSMGR_SRC}
							${GAMELIB_PLAYSURFACEMGR_SRC}
							${GAMELIB_RAGDOLL_SRC}
							${GAMELIB_RESOURCEMGR_SRC}
							${GAMELIB_SOUND_SRC}
							${GAMELIB_SPATIALDBASE_SRC}
							${GAMELIB_STATEMGR_SRC}
							${GAMELIB_STRINGMGR_SRC}
							${GAMELIB_TEMPLATEMGR_SRC}
							${GAMELIB_TRACERS_SRC}
							${GAMELIB_TRIGGEREX_SRC}
							${GAMELIB_TWEAKMGR_SRC}
							${GAMELIB_ZONEMGR_SRC}
)

set_support_defaults(GameLib)

target_compile_definitions(GameLib PRIVATE DISABLE_LEGACY_CODE)

target_link_libraries(GameLib 	Entropy 
								MiscUtils 
								ConversationMgr 
								EventMgr 
								fx_RunTime 
								Parsing 
								audiomgr
								Render
								NetworkMgr
								UI
								UIDialogs
								MusicMgr
)